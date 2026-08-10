import csv
import io
import os
import threading
from datetime import datetime

import webview
from dotenv import load_dotenv
from flask import Flask, Response, jsonify, request, send_from_directory
from pymongo import ASCENDING, MongoClient
from pymongo.errors import ServerSelectionTimeoutError

load_dotenv()

PORT = 8000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# Defaults to a local MongoDB instance (mongodb://localhost:27017). Copy
# .env.example to .env and set MONGODB_URI there to point at Atlas or
# another server instead.
MONGODB_URI = os.environ.get("MONGODB_URI", "mongodb://localhost:27017")

mongo_client = MongoClient(MONGODB_URI, serverSelectionTimeoutMS=5000)
db = mongo_client["nicla_sense_db"]
readings = db["sensor_readings"]

try:
    readings.create_index([("session_id", ASCENDING), ("timestamp", ASCENDING)])
except ServerSelectionTimeoutError as exc:
    raise RuntimeError(
        f"Could not reach MongoDB at '{MONGODB_URI}'. If you're using local "
        "MongoDB, make sure the 'MongoDB' Windows service is running "
        "(services.msc, or `net start MongoDB` as Administrator). If you meant "
        "to use Atlas, set MONGODB_URI in .env."
    ) from exc

app = Flask(__name__, static_folder=None)


@app.route("/")
@app.route("/<path:filename>")
def serve_static(filename="index.html"):
    return send_from_directory(DIRECTORY, filename)


@app.route("/api/readings", methods=["POST"])
def create_reading():
    payload = request.get_json(force=True, silent=True) or {}

    session_id = payload.get("session_id")
    timestamp = payload.get("timestamp")
    if not session_id or not timestamp:
        return jsonify({"error": "session_id and timestamp are required"}), 400

    document = {
        "session_id": session_id,
        "timestamp": datetime.fromisoformat(timestamp.replace("Z", "+00:00")),
        "accelerometer": payload.get("accelerometer"),
        "gyroscope": payload.get("gyroscope"),
        "temperature": payload.get("temperature"),
        "analog_a1": payload.get("analog_a1"),
    }
    readings.insert_one(document)
    return jsonify({"status": "ok"}), 201


@app.route("/api/export", methods=["GET"])
def export_csv():
    # session_id is optional: with it, export just that pairing session;
    # without it (e.g. no device currently paired), export everything logged.
    session_id = request.args.get("session_id")
    query = {"session_id": session_id} if session_id else {}
    cursor = readings.find(query).sort("timestamp", ASCENDING)

    buffer = io.StringIO()
    writer = csv.writer(buffer)
    writer.writerow([
        "timestamp", "accel_x", "accel_y", "accel_z",
        "gyro_x", "gyro_y", "gyro_z", "temperature", "analog_a1",
    ])

    for doc in cursor:
        accel = doc.get("accelerometer") or {}
        gyro = doc.get("gyroscope") or {}
        writer.writerow([
            doc["timestamp"].isoformat(),
            accel.get("x"), accel.get("y"), accel.get("z"),
            gyro.get("x"), gyro.get("y"), gyro.get("z"),
            doc.get("temperature"), doc.get("analog_a1"),
        ])

    filename = f"NiclaData_{session_id}.csv" if session_id else "NiclaData_all.csv"
    return Response(
        buffer.getvalue(),
        mimetype="text/csv",
        headers={"Content-Disposition": f"attachment; filename={filename}"},
    )


def start_server():
    app.run(host="127.0.0.1", port=PORT, threaded=True, use_reloader=False)


if __name__ == "__main__":
    server_thread = threading.Thread(target=start_server, daemon=True)
    server_thread.start()

    webview.create_window(
        "Arduino BLE Dashboard",
        f"http://127.0.0.1:{PORT}/index.html",
        width=1280,
        height=720,
    )
    webview.start()
