function Component() {
    // Constructor
}

var uninstallRegistryKey = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CodexBarX";

function addRegistrySetOperation(name, type, value) {
    component.addOperation("Execute", [
        "reg.exe",
        "ADD",
        uninstallRegistryKey,
        "/v",
        name,
        "/t",
        type,
        "/d",
        value,
        "/f",
        "UNDOEXECUTE",
        "reg.exe",
        "DELETE",
        uninstallRegistryKey,
        "/v",
        name,
        "/f"
    ]);
}

function addUninstallRegistryOperations() {
    addRegistrySetOperation("DisplayName", "REG_SZ", "CodexBarX");
    addRegistrySetOperation("DisplayVersion", "REG_SZ", "@Version@");
    addRegistrySetOperation("Publisher", "REG_SZ", "CodexBarX");
    addRegistrySetOperation("UninstallString", "REG_SZ", "\"@TargetDir@/@MaintenanceToolName@.exe\"");
    addRegistrySetOperation("InstallLocation", "REG_SZ", "@TargetDir@");
    addRegistrySetOperation("DisplayIcon", "REG_SZ", "\"@TargetDir@/CodexBarX.exe\",0");
    addRegistrySetOperation("NoModify", "REG_DWORD", "1");
    addRegistrySetOperation("NoRepair", "REG_DWORD", "1");
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        if (installer.value("os") === "win") {
            // Create desktop shortcut
            component.addOperation("CreateShortcut",
                "@TargetDir@/CodexBarX.exe",
                "@DesktopDir@/CodexBarX.lnk",
                "workingDirectory=@TargetDir@,iconPath=@TargetDir@/CodexBarX.exe,iconId=0"
            );

            // Create start menu shortcut
            component.addOperation("CreateShortcut",
                "@TargetDir@/CodexBarX.exe",
                "@StartMenuDir@/CodexBarX.lnk",
                "workingDirectory=@TargetDir@,iconPath=@TargetDir@/CodexBarX.exe,iconId=0"
            );

            // Create uninstall shortcut in start menu
            component.addOperation("CreateShortcut",
                "@TargetDir@/@MaintenanceToolName@.exe",
                "@StartMenuDir@/Uninstall CodexBarX.lnk",
                "workingDirectory=@TargetDir@"
            );

            // Register uninstaller in Windows Add/Remove Programs
            component.addOperation("RegisterFileType",
                "CodexBarX.Assoc",
                "@TargetDir@/CodexBarX.exe",
                "CodexBarX Application"
            );

            addUninstallRegistryOperations();
        }
    } catch (e) {
        print("Error in createOperations: " + e);
    }
};

Component.prototype.createOperationsForArchive = function(archive) {
    component.addOperation("Extract", archive, "@TargetDir@");
};

Component.prototype.createOperationsForPath = function(path) {
    component.addOperation("Copy", path, "@TargetDir@/" + path);
};
