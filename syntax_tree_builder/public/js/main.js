const { app, BrowserWindow, Menu, ipcMain } = require('electron');
const url = require('url');
const path = require('path');
const fs = require('fs');
let mainWindow;
// Listen for the app to be ready
app.on('ready', () => {
    // Create new window
    mainWindow = new BrowserWindow({
        width: 1280,
        height: 720,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
            // enableRemoteModule: true
        },
        resizable: false
    });
    // Load html file into the window
    mainWindow.loadURL(url.format({
        pathname: path.join(__dirname, '../html/index.html'),
        protocol: 'file',
        slashes: true
    }));
    // Quite app when closed
    mainWindow.on('closed', () => {
        app.quit();
    });
});
ipcMain.on("Syntax:window_ready", () => {
    const appPath = app.isPackaged ? path.dirname(app.getPath('exe')) : __dirname;
    const filePath = app.isPackaged ? '../../SyntaxTree.txt' : '../../SyntaxTree.txt';
    fs.readFile(path.join(appPath, filePath), 'utf8', (err, data) => {
        if (err) {
            console.log(err);
            return;
        }
        mainWindow.webContents.send("Syntax:data", data, appPath);
    });
});
