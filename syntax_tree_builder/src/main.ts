const { app, BrowserWindow, Menu, ipcMain} = require('electron');
const url = require('url');
const path = require('path');
const fs = require('fs');

let mainWindow;

// Listen for the app to be ready
app.on('ready', (): void => {
    // Create new window
    mainWindow = new BrowserWindow({
        width: 1280,
        height: 720,
        webPreferences:{
            nodeIntegration:true,
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
    mainWindow.on('closed', (): void => {
        app.quit();
    });
});

ipcMain.on("Syntax:window_ready", () => {
    const appPath = app.isPackaged ? path.dirname(app.getPath('exe')) : app.getAppPath(); app.isPackaged ? path.dirname(app.getPath('exe')) : app.getAppPath();
    fs.readFile(path.join(appPath, '../SyntaxTree.txt'), 'utf8', (err, data) => {
        if (err) {
            console.log(err);
            return;
        }
        mainWindow.webContents.send("Syntax:data", data, appPath);
    });
});




