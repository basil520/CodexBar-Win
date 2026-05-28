var installerTheme = {
    title: "#F7FBFF",
    body: "#C9D8EA",
    muted: "#9FB2C8",
    accent: "#49A3B0",
    warning: "#F6C96B"
};

var installerPageCopies = [
    {
        id: QInstaller.Introduction,
        objectName: "IntroductionPage",
        title: "安装 CodexBarX",
        subtitle: "轻量托盘、用量追踪和多 Provider 状态看板，一次完成安装。"
    },
    {
        id: QInstaller.TargetDirectory,
        objectName: "TargetDirectoryPage",
        title: "选择安装位置",
        subtitle: "选择 CodexBarX 主程序和运行文件的保存目录。"
    },
    {
        id: QInstaller.ComponentSelection,
        objectName: "ComponentSelectionPage",
        title: "选择安装组件",
        subtitle: "安装 CodexBarX 桌面应用及其必要运行文件。"
    },
    {
        id: QInstaller.LicenseCheck,
        objectName: "LicenseAgreementPage",
        title: "阅读许可协议",
        subtitle: "继续安装前，请阅读并接受 CodexBarX 的许可条款。"
    },
    {
        id: QInstaller.StartMenuSelection,
        objectName: "StartMenuDirectoryPage",
        title: "开始菜单文件夹",
        subtitle: "选择 CodexBarX 快捷方式在开始菜单中的位置。"
    },
    {
        id: QInstaller.ReadyForInstallation,
        objectName: "ReadyForInstallationPage",
        title: "准备开始安装",
        subtitle: "确认安装位置、组件和快捷方式设置。"
    },
    {
        id: QInstaller.PerformInstallation,
        objectName: "PerformInstallationPage",
        title: "正在安装 CodexBarX",
        subtitle: "正在复制文件、写入配置并准备快捷方式。"
    },
    {
        id: QInstaller.InstallationFinished,
        objectName: "FinishedPage",
        title: "CodexBarX 已准备就绪",
        subtitle: "安装完成后，可以立即启动或稍后从开始菜单打开。"
    }
];

function Controller() {
    applyWizardButtonText();
}

function applyWizardButtonText() {
    for (var i = 0; i < installerPageCopies.length; i++) {
        var pageId = installerPageCopies[i].id;
        setWizardButtonText(pageId, buttons.BackButton, "上一步");
        setWizardButtonText(pageId, buttons.NextButton, "下一步");
        setWizardButtonText(pageId, buttons.CommitButton, "安装");
        setWizardButtonText(pageId, buttons.FinishButton, "完成");
        setWizardButtonText(pageId, buttons.CancelButton, "取消");
    }
}

function setWizardButtonText(pageId, buttonId, text) {
    try {
        gui.setWizardPageButtonText(pageId, buttonId, text);
    } catch (e) {
        print("Unable to set wizard button text: " + e);
    }
}

function htmlTitle(text) {
    return "<h2 style='color:" + installerTheme.title + "; margin:0 0 8px 0; font-size:22px;'>" + text + "</h2>";
}

function htmlLead(text) {
    return "<p style='color:" + installerTheme.body + "; margin:0 0 12px 0; line-height:145%;'>" + text + "</p>";
}

function htmlNote(text) {
    return "<p style='color:" + installerTheme.muted + "; margin:0; line-height:145%;'>" + text + "</p>";
}

function htmlFeature(title, body) {
    return "<p style='color:" + installerTheme.title + "; margin:10px 0 0 0;'>" +
        "<span style='color:" + installerTheme.accent + ";'><b>" + title + "</b></span><br/>" +
        "<span style='color:" + installerTheme.muted + "; line-height:145%;'>" + body + "</span>" +
        "</p>";
}

function htmlWarning(text) {
    return "<p style='color:" + installerTheme.warning + "; margin:10px 0 0 0; line-height:145%;'>" + text + "</p>";
}

function applyPageCopy(objectName, title, subtitle) {
    var page = null;
    try {
        page = gui.pageByObjectName(objectName);
    } catch (e) {
        print("Unable to resolve installer page '" + objectName + "': " + e);
    }

    if (!page) {
        return;
    }

    try {
        page.title = title;
    } catch (titlePropertyError) {
        try {
            page.setTitle(title);
        } catch (titleMethodError) {
            print("Unable to set title for '" + objectName + "': " + titleMethodError);
        }
    }

    try {
        page.subTitle = subtitle;
    } catch (subtitlePropertyError) {
        try {
            page.setSubTitle(subtitle);
        } catch (subtitleMethodError) {
            print("Unable to set subtitle for '" + objectName + "': " + subtitleMethodError);
        }
    }
}

function applyPageCopyByName(objectName) {
    for (var i = 0; i < installerPageCopies.length; i++) {
        var pageCopy = installerPageCopies[i];
        if (pageCopy.objectName === objectName) {
            applyPageCopy(pageCopy.objectName, pageCopy.title, pageCopy.subtitle);
            return;
        }
    }
}

function setPageLabelText(objectName, labelName, text) {
    var widget = null;
    try {
        widget = gui.pageWidgetByObjectName(objectName);
    } catch (e) {
        print("Unable to resolve installer page widget '" + objectName + "': " + e);
    }

    if (!widget || !widget[labelName]) {
        return;
    }

    try {
        widget[labelName].setText(text);
    } catch (textError) {
        print("Unable to set " + objectName + "." + labelName + ": " + textError);
    }

    try {
        widget[labelName].setWordWrap(true);
    } catch (wrapError) {
        // Some IFW labels do not expose word wrapping through scripting.
    }
}

Controller.prototype.IntroductionPageCallback = function() {
    applyPageCopyByName("IntroductionPage");
    setPageLabelText(
        "IntroductionPage",
        "MessageLabel",
        htmlTitle("安装 CodexBarX") +
        htmlLead("CodexBarX 会在桌面侧边保持一个轻量托盘入口，集中查看 AI 编码工具的用量、余额和 Provider 状态。") +
        htmlFeature("轻量托盘", "安装后可以常驻托盘，快速打开用量面板和设置页。") +
        htmlFeature("用量追踪", "按日、周、月展示关键 Provider 的消耗与剩余额度。") +
        htmlFeature("多 Provider", "统一整理 Codex、Claude、Kimi、QianFan 等来源的状态信息。")
    );
};

Controller.prototype.TargetDirectoryPageCallback = function() {
    applyPageCopyByName("TargetDirectoryPage");
    setPageLabelText(
        "TargetDirectoryPage",
        "MessageLabel",
        htmlTitle("选择安装位置") +
        htmlLead("建议保留默认目录，安装器会把 CodexBarX 主程序、运行文件和卸载信息放在同一位置。") +
        htmlFeature("默认路径", "适合大多数用户，也便于后续更新或卸载。") +
        htmlWarning("如果选择受保护目录，Windows 可能会请求管理员权限。")
    );
};

Controller.prototype.ComponentSelectionPageCallback = function() {
    applyPageCopyByName("ComponentSelectionPage");
    setPageLabelText(
        "ComponentSelectionPage",
        "MessageLabel",
        htmlTitle("选择安装组件") +
        htmlLead("默认组件包含 CodexBarX 桌面应用和运行所需文件。") +
        htmlNote("除非你明确知道要裁剪哪些组件，否则建议保持默认选择。")
    );
};

Controller.prototype.LicenseAgreementPageCallback = function() {
    applyPageCopyByName("LicenseAgreementPage");
    setPageLabelText(
        "LicenseAgreementPage",
        "LicenseInfoLabel",
        "请阅读许可协议。接受条款后，才能继续安装 CodexBarX。"
    );
    setPageLabelText(
        "LicenseAgreementPage",
        "AcceptLicenseLabel",
        "我已阅读并接受 CodexBarX 许可协议。"
    );
};

Controller.prototype.StartMenuDirectoryPageCallback = function() {
    applyPageCopyByName("StartMenuDirectoryPage");
    setPageLabelText(
        "StartMenuDirectoryPage",
        "MessageLabel",
        htmlTitle("设置开始菜单快捷方式") +
        htmlLead("安装器会在指定文件夹中创建 CodexBarX 快捷方式，方便之后从开始菜单启动。") +
        htmlNote("如果你不需要自定义分组，可以保持默认名称。")
    );
};

Controller.prototype.ReadyForInstallationPageCallback = function() {
    applyPageCopyByName("ReadyForInstallationPage");
    setPageLabelText(
        "ReadyForInstallationPage",
        "MessageLabel",
        htmlTitle("准备开始安装") +
        htmlLead("安装器已经收集好所需信息。点击“安装”后，将开始复制文件并创建快捷方式。") +
        htmlFeature("安装内容", "CodexBarX 主程序、运行文件、卸载入口和开始菜单快捷方式。")
    );
};

Controller.prototype.PerformInstallationPageCallback = function() {
    applyPageCopyByName("PerformInstallationPage");
    setPageLabelText(
        "PerformInstallationPage",
        "MessageLabel",
        htmlTitle("正在安装") +
        htmlLead("CodexBarX 文件正在复制，相关快捷方式和卸载信息正在写入。") +
        htmlNote("这个过程通常只需要几秒钟。")
    );
};

Controller.prototype.FinishedPageCallback = function() {
    applyPageCopyByName("FinishedPage");
    setPageLabelText(
        "FinishedPage",
        "MessageLabel",
        htmlTitle("安装完成") +
        htmlLead("CodexBarX 已成功安装，可以立即启动，也可以稍后从开始菜单或桌面入口打开。") +
        htmlFeature("下一步", "打开设置页配置 Provider 凭据和显示偏好，然后让托盘面板保持在你顺手的位置。")
    );
};
