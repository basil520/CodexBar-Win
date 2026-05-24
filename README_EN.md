# CodexBarX

English | [简体中文](README.md)

**CodexBarX** is a Windows system tray utility for real-time monitoring of usage, quotas, and reset times across multiple AI coding assistants. Built with C++ and Qt 6.5, it's lightweight, efficient, and perfect for developers who need persistent desktop monitoring.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.5+-green.svg)

---

![Tray Usage Panel](resources/screenshots/tray-usage.png)

![Provider Settings](resources/screenshots/settings-providers.png)

---

## Features

### Core Features

- **Real-time Usage Monitoring** — Display today's, 30-day, session, and weekly usage statistics in the system tray
- **30+ Provider Support** — Covering all major AI coding assistants, see the full list below
- **Quota Tracking** — Display remaining quota, progress bars, and estimated depletion time
- **Reset Reminders** — Show countdown to quota reset
- **Connection Status** — Real-time connection status for each provider
- **Auto-start** — Support automatic startup on boot
- **Bilingual** — Full internationalization support for Chinese and English

### Visual & Interaction

- **Three Themes** — Dark / Midnight Blue / Amethyst
- **Acrylic Glass Effect** — Native Windows DWM blur with adjustable opacity (5%–95%)
- **Ambient Animations** — Brand-colored flowing gradient backgrounds with three quality tiers (High / Balanced / Low)
- **Accessibility** — Reduce Motion mode minimizes decorative animations while preserving essential feedback
- **Command Palette** — Ctrl+P to quickly search providers and actions
- **Elastic Micro-interactions** — Hover scale, click bounce, expand/collapse smooth transitions

### Data & Charts

- **Cost History Chart** — Daily cost bar chart with peak markers and model-level cost breakdown
- **Credits History Chart** — Daily credits-used bar chart
- **Usage Breakdown Chart** — Stacked bar chart by service category (CLI / GitHub Review / API / Codex / Storage, etc.)
- **Plan Utilization Chart** — Plan utilization curve over time
- **Storage Analysis** — Per-path byte breakdown with cleanup suggestions

### Multi-Account Management

- **Codex Multi-account** — OAuth device flow authentication, account switching, upgrade/downgrade, re-authentication
- **Token Multi-account** — API Key management and quick switching
- **Account Reconciliation** — Automatic sync between local and remote account state

### Browser Session Bridge

Import browser login credentials directly via a Chrome extension + WebSocket local server, bypassing the Chrome 127+ encrypted cookie database limitation:

- Supports Chrome / Edge / Brave / Opera / Vivaldi
- Three import modes: Cookie / localStorage / Hybrid
- Auto-matches domains and cookie names per provider
- Browser Profile binding with auto-sync
- Secure design: Bearer Token auto-redaction, minimal extension permissions

### Tray Customization

- **Four Tray Display Modes** — Icon Only / Percentage / Remaining Time / Custom Time
- **Merge Icons** — Combine all enabled providers into a single tray icon
- **Usage Bar Direction** — Toggle between showing used percentage or remaining percentage
- **Threshold Alerts** — Configurable Warning (5%–95%) and Critical (1%–50%) threshold sliders
- **Absolute Reset Times** — Toggle between exact reset timestamps and relative wording
- **Session Quota Notifications** — Alert when provider session quota becomes constrained

### Diagnostics & Debugging

- **Provider Status Checks** — Poll statuspage endpoints for service health monitoring
- **Debug Panel** — One-click refresh, clear cache, reset settings, connection test
- **Verbose Logging** — Strategy-level diagnostics during Codex fetches
- **Web Debug Dump** — Save raw HTML for offline troubleshooting

---

## Supported AI Providers

### Major Providers

| Provider | Authentication | Features |
|----------|---------------|----------|
| **OpenAI Codex** | OAuth / CLI | ✅ Quota, Usage, Reset Time, Multi-account |
| **Claude** | API Key | ✅ Usage Statistics, Peak Pricing Indicator |
| **GitHub Copilot** | OAuth | ✅ Subscription Status |
| **Cursor** | Cookie | ✅ Quota, Usage |
| **Kimi** | Cookie / Token | ✅ Usage Statistics |
| **DeepSeek** | API Key | ✅ Usage Statistics |

### API Platforms

| Platform | Authentication | Features |
|----------|---------------|----------|
| **OpenRouter** | API Key | ✅ Usage, Balance, Key Quota |
| **z.ai** | Cookie | ✅ Quota, Usage |
| **Perplexity** | API Key | ✅ Usage Statistics |
| **Mistral** | API Key | ✅ Usage Statistics |

### Other Providers

| Provider | Authentication | Provider | Authentication |
|----------|---------------|----------|---------------|
| Google Gemini | API Key | Google VertexAI | Service Account |
| MiniMax | API Key | Alibaba (Tongyi Qianwen) | API Key |
| Ollama | Local Service | OpenCode | Local SQLite |
| Augment | Cookie | Amp | Cookie |
| JetBrains AI | Cookie | Factory | API Key |
| Warp | Cookie | Abacus | Cookie |
| Codebuff | Cookie | Windsurf | SQLite |
| KimiK2 | Cookie | Kilo | Cookie |
| Kiro | Cookie | Antigravity | Cookie |
| QianFan | API Key | Venice | API Key |
| Manus | Cookie | Doubao | Cookie |
| StepFun | API Key | Mimo | Cookie |
| XFXinChen | Cookie | OpenAI API | API Key |
| Crof | Cookie | CommandCode | Cookie |
| Synthetic | API Key | OpenCodeGo | Local SQLite |

---

## System Requirements

### Runtime

- **Operating System**: Windows 10 1809 or later
- **Runtime**: No additional dependencies required (all bundled)

### Development Environment (Build Only)

- **Compiler**: Visual Studio 2022 or Build Tools
- **Qt**: 6.5.3 or later (MSVC 2022 64-bit)
- **CMake**: 3.21 or later
- **Git**: For cloning the repository

---

## Installation

### Method 1: Download Installer (Recommended)

1. Visit the [Releases](https://github.com/basil520/CodexBarX/releases) page
2. Download the latest `CodexBarX-x.x.x-Installer.exe`
3. Run the installer and follow the prompts
4. Desktop shortcut and Start Menu entry will be created automatically

### Method 2: Portable Version

1. Download `CodexBarX-x.x.x-portable.zip`
2. Extract to any directory
3. Double-click `CodexBarX.exe` to run
4. Optionally create a desktop shortcut for convenience

---

## Usage

### First Run

1. Launch CodexBarX, it will minimize to the system tray
2. Right-click the tray icon and select **Settings**
3. In the **Providers** tab, enable the providers you need
4. Configure authentication as prompted (API Key or auto-login)

### Configuring Providers

#### API Key Method

1. Select the provider in Settings
2. Click **Configure**
3. Enter your API Key
4. Click **Test** to verify the connection

#### Auto-login Method

Some providers support automatic reading of browser cookies or CLI configurations:

- **OpenAI Codex**: Automatically reads Codex CLI config or browser login status
- **Cursor**: Automatically reads Chrome/Edge cookies
- **Kimi**: Automatically reads Chrome/Edge cookies

#### Browser Session Bridge (Advanced Alternative for Cookie Method)

For providers where Chrome 127+ encrypted cookie database cannot be read directly:

1. Enable **Browser Session Bridge** in **Advanced** settings
2. Follow the guide to install the CodexBarX browser extension (supports Chrome / Edge / Brave / Opera / Vivaldi)
3. Bind browser profiles to each provider
4. Enable auto-sync or manually click import

### Viewing Usage

- Left-click the tray icon to expand the usage panel
- Displays today's usage, 30-day usage, session usage, etc.
- Progress bar shows remaining quota percentage
- Hover to see estimated depletion time and reset time

### Advanced Settings

- **Auto Refresh**: Set refresh interval (default: 5 minutes)
- **Auto-start on Boot**: Configure automatic startup
- **Theme**: Dark / Midnight Blue / Amethyst
- **Acrylic Effect**: Toggle and opacity slider
- **Visual Quality**: High / Balanced / Low
- **Reduce Motion**: Minimize decorative animations
- **Language**: Chinese and English interfaces

---

## Building from Source

### Clone Repository

```powershell
git clone https://github.com/basil520/CodexBarX.git
cd CodexBarX
```

### Install Dependencies

1. Install Visual Studio 2022 or Build Tools
2. Install Qt 6.5.3 (MSVC 2022 64-bit)
3. Set the `CMAKE_PREFIX_PATH` environment variable to your Qt installation

```powershell
# Example
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.5.3\msvc2019_64"
```

### Build

```powershell
# Configure project
cmake -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.5.3\msvc2019_64

# Build
cmake --build build --config Release --parallel

# Run
.\build\Release\CodexBarX.exe
```

### Build Installer

```powershell
# Install Qt Installer Framework
# Download: https://download.qt.io/official_releases/qt-installer-framework/

# Build installer
.\Scripts\build-installer.ps1 -Version 0.1.0
```

---

## Testing

The project includes a comprehensive unit test suite:

```powershell
# Build project with tests
cmake -B build -DBUILD_TESTS=ON

# Run all tests
ctest --test-dir build -C Release --output-on-failure

# Run specific test
ctest --test-dir build -C Release -R tst_RateWindow
```

---

## Project Structure

```
CodexBarX/
├── src/                    # Source code
│   ├── app/               # Application core logic
│   ├── providers/         # Provider implementations (30+)
│   ├── models/            # Data models
│   ├── network/           # Network layer
│   ├── tray/              # Tray functionality
│   ├── browserbridge/     # Browser session bridge
│   ├── account/           # Multi-account management
│   └── util/              # Utilities
├── qml/                    # QML interfaces
│   ├── components/        # Common components
│   ├── panes/             # Settings pages
│   └── provider/          # Provider detail components
├── resources/              # Resource files
│   ├── icons/             # Provider icons
│   ├── screenshots/       # Screenshots
│   └── browser-session-bridge/  # Chrome extension
├── translations/           # Translation files
├── tests/                  # Unit tests
├── installer/              # Installer configuration
├── Scripts/                # Build scripts
└── .github/workflows/      # GitHub Actions configuration
```

---

## FAQ

### Q: Why can't I see usage data?

**A:** Please check:
1. Is the provider enabled?
2. Is the authentication information correct?
3. Is the network connection working?
4. Try clicking the refresh button manually

### Q: How to add multiple accounts?

**A:** Some providers support multiple accounts:
1. Add a new provider instance in Settings
2. Configure different authentication credentials
3. Rename to distinguish different accounts

### Q: What if cookies can't be read after Chrome updates?

**A:** Chrome 127+ encrypts the cookie database. Use the **Browser Session Bridge** feature:
1. Enable Browser Session Bridge in Advanced settings
2. Install the browser extension and bind a Profile
3. Cookies will then be securely imported through the extension

### Q: Where is data stored?

**A:**
- Configuration: `%APPDATA%\CodexBar\`
- Credentials: Windows Credential Manager (encrypted)
- Logs: `%LOCALAPPDATA%\CodexBar\logs\`

### Q: Does it support proxies?

**A:** CodexBarX uses system proxy settings and automatically reads Windows proxy configuration.

---

## Contributing

Contributions are welcome! Feel free to submit code, report issues, or suggest features!

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Standards

- Follow C++17 standard
- Use Qt coding style
- Add unit tests
- Update documentation

---

## Attribution

This project is based on [CodexBar](https://github.com/steipete/CodexBar) with Windows platform adaptation and independent maintenance.

Special thanks to the original author [@steipete](https://github.com/steipete) for the open-source contribution!

---

## License

This project is open-sourced under the [MIT License](LICENSE).

---

## Contact

- **Bug Reports**: [GitHub Issues](https://github.com/basil520/CodexBarX/issues)
- **Feature Requests**: [GitHub Discussions](https://github.com/basil520/CodexBarX/discussions)

---

<p align="center">
  Made with ❤️ for AI developers
</p>