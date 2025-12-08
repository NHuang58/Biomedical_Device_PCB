const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("electronAPI", {
  saveData: (data) => ipcRenderer.send("save-data", data)
});
