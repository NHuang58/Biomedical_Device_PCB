const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');

function createWindow() {
  const win = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      enableBlinkFeatures: "WebBluetooth",
      contextIsolation: true,
      nodeIntegration: false,
    }
  });

  win.loadFile("app.html");
}

app.whenReady().then(createWindow);

ipcMain.on("save-data", (event, data) => {
  const output = path.join(__dirname, "nicla_data.json");
  fs.writeFileSync(output, JSON.stringify(data, null, 2));
  console.log("Saved →", output);
});
