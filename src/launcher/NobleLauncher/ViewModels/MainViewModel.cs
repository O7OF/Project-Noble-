using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Input;
using Microsoft.Win32;
using NobleLauncher.Models;
using NobleLauncher.Services;

namespace NobleLauncher.ViewModels
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private readonly GameDiscoveryService _discoveryService;
        private readonly ConfigurationService _configService;
        private readonly ModManager _modManager;
        private readonly ProcessSupervisor _processSupervisor;

        private GameInstallation _installation = new();
        private LauncherConfig _config = new();
        private string _statusText = "Initializing...";
        private string _logOutput = string.Empty;
        private bool _isBusy;

        public event PropertyChangedEventHandler? PropertyChanged;

        public GameInstallation Installation
        {
            get => _installation;
            set { _installation = value; OnPropertyChanged(); }
        }

        public LauncherConfig Config
        {
            get => _config;
            set { _config = value; OnPropertyChanged(); }
        }

        public string StatusText
        {
            get => _statusText;
            set { _statusText = value; OnPropertyChanged(); }
        }

        public string LogOutput
        {
            get => _logOutput;
            set { _logOutput = value; OnPropertyChanged(); }
        }

        public bool IsBusy
        {
            get => _isBusy;
            set { _isBusy = value; OnPropertyChanged(); }
        }

        private FirefightBlockInfo? _selectedFirefightMission;
        private string _selectedDifficulty = "Heroic";
        private string _selectedCampaignMission = "Winter Contingency";

        public ObservableCollection<ModPackage> Mods { get; } = new();
        public ObservableCollection<FirefightBlockInfo> FirefightMissions { get; } = new();
        public ObservableCollection<string> Difficulties { get; } = new() { "Easy", "Normal", "Heroic", "Legendary" };
        public ObservableCollection<string> CampaignMissions { get; } = new()
        {
            "Noble Actual",
            "Winter Contingency",
            "ONI: Sword Base",
            "Nightfall",
            "Tip of the Spear",
            "Long Night of Solace",
            "Exodus",
            "New Alexandria",
            "The Package",
            "The Pillar of Autumn",
            "Lone Wolf"
        };

        public FirefightBlockInfo? SelectedFirefightMission
        {
            get => _selectedFirefightMission;
            set { _selectedFirefightMission = value; OnPropertyChanged(); }
        }

        public string SelectedDifficulty
        {
            get => _selectedDifficulty;
            set { _selectedDifficulty = value; OnPropertyChanged(); }
        }

        public string SelectedCampaignMission
        {
            get => _selectedCampaignMission;
            set { _selectedCampaignMission = value; OnPropertyChanged(); }
        }

        public ICommand LaunchGameCommand { get; }
        public ICommand LaunchMainMenuCommand { get; }
        public ICommand LaunchFirefightCommand { get; }
        public ICommand LaunchCampaignCommand { get; }
        public ICommand LaunchDedicatedCommand { get; }
        public ICommand BrowseMccPathCommand { get; }
        public ICommand RefreshDiscoveryCommand { get; }
        public ICommand RefreshModsCommand { get; }
        public ICommand SaveSettingsCommand { get; }
        public ICommand ClearLogCommand { get; }

        public MainViewModel()
        {
            _discoveryService = new GameDiscoveryService();
            _configService = new ConfigurationService();
            _modManager = new ModManager();
            _processSupervisor = new ProcessSupervisor();

            _processSupervisor.LogMessageReceived += msg =>
            {
                App.Current?.Dispatcher.Invoke(() =>
                {
                    LogOutput += msg + Environment.NewLine;
                });
            };

            _processSupervisor.ProcessExited += code =>
            {
                App.Current?.Dispatcher.Invoke(() =>
                {
                    StatusText = $"Engine session ended (Code: {code}). Ready.";
                    IsBusy = false;
                });
            };

            LaunchGameCommand = new RelayCommand(_ => LaunchMainMenu(), _ => !IsBusy && Installation.IsValid);
            LaunchMainMenuCommand = new RelayCommand(_ => LaunchMainMenu(), _ => !IsBusy && Installation.IsValid);
            LaunchFirefightCommand = new RelayCommand(_ => LaunchFirefight(), _ => !IsBusy && Installation.IsValid && SelectedFirefightMission != null);
            LaunchCampaignCommand = new RelayCommand(_ => LaunchCampaign(), _ => !IsBusy && Installation.IsValid);
            LaunchDedicatedCommand = new RelayCommand(_ => LaunchDedicated(), _ => !IsBusy && Installation.IsValid);
            BrowseMccPathCommand = new RelayCommand(_ => BrowseMccPath());
            RefreshDiscoveryCommand = new RelayCommand(_ => RunDiscovery());
            RefreshModsCommand = new RelayCommand(_ => RefreshMods());
            SaveSettingsCommand = new RelayCommand(_ => SaveSettings());
            ClearLogCommand = new RelayCommand(_ => LogOutput = string.Empty);

            Initialize();
        }

        private void Initialize()
        {
            AppendLog("Starting Project Noble Launcher Hub...");
            Config = _configService.Load();
            RunDiscovery();
            RefreshMods();
            RefreshFirefightBlocks();
        }

        public void RunDiscovery()
        {
            AppendLog("Scanning for Halo: The Master Chief Collection (AppID 976730)...");
            Installation = _discoveryService.DiscoverInstallation(Config.CustomMccPath);

            if (Installation.IsValid)
            {
                StatusText = $"Halo: Reach DLC verified ({Installation.MapCount} maps, haloreach.dll ready).";
                AppendLog($"[SUCCESS] Found MCC Root: {Installation.MccRootPath}");
                AppendLog($"[SUCCESS] Found haloreach.dll ({Installation.ReachDllSizeBytes / (1024 * 1024)} MB)");
                AppendLog($"[SUCCESS] Found Maps Directory: {Installation.MapsDirectory}");
                if (!string.IsNullOrEmpty(Installation.WorkshopDirectory))
                {
                    AppendLog($"[INFO] Steam Workshop Directory: {Installation.WorkshopDirectory}");
                }
            }
            else
            {
                StatusText = Installation.StatusMessage;
                AppendLog($"[WARNING] {Installation.StatusMessage}");
            }
        }

        public void RefreshMods()
        {
            Mods.Clear();
            var scanned = _modManager.ScanMods(Installation.WorkshopDirectory, Config.CustomDataDirectory);
            foreach (var mod in scanned)
            {
                Mods.Add(mod);
            }
            AppendLog($"Indexed {Mods.Count} mods ({Mods.Count(m => m.Source == ModSource.SteamWorkshop)} Workshop, {Mods.Count(m => m.Source == ModSource.LocalDirectory)} Local).");
        }

        public void RefreshFirefightBlocks()
        {
            FirefightMissions.Clear();
            string blocksDir = Path.Combine(Directory.GetCurrentDirectory(), "data", "blocks");
            if (!Directory.Exists(blocksDir))
            {
                blocksDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data", "blocks");
            }

            var scanned = FirefightBlockDecoder.ScanDirectory(blocksDir);
            // Deduplicate to show one entry per map (defaulting to Normal d1)
            var seenMaps = new HashSet<int>();
            foreach (var block in scanned)
            {
                if (block.Difficulty == 1 && !seenMaps.Contains(block.MapId))
                {
                    seenMaps.Add(block.MapId);
                    FirefightMissions.Add(block);
                }
            }

            // Fallback if no d1 found
            if (FirefightMissions.Count == 0)
            {
                foreach (var block in scanned)
                {
                    if (!seenMaps.Contains(block.MapId))
                    {
                        seenMaps.Add(block.MapId);
                        FirefightMissions.Add(block);
                    }
                }
            }

            if (FirefightMissions.Count > 0)
            {
                SelectedFirefightMission = FirefightMissions[0];
            }

            AppendLog($"Decoded {FirefightMissions.Count} Firefight missions from {blocksDir}.");
        }

        private void LaunchMainMenu()
        {
            IsBusy = true;
            StatusText = "Launching Halo Reach Main Menu...";
            AppendLog("Launching interactive 3D Main Menu (mainmenu_all.map)...");

            _modManager.SaveModState(Mods);
            _configService.Save(Config);

            bool launched = _processSupervisor.Launch(Config, Installation, dedicated: false, customMap: "mainmenu", customMode: "mainmenu");
            if (!launched)
            {
                IsBusy = false;
                StatusText = "Launch failed. Check log for details.";
            }
            else
            {
                StatusText = "noble-client.exe running (Main Menu).";
            }
        }

        private void LaunchFirefight()
        {
            if (SelectedFirefightMission == null) return;

            IsBusy = true;
            int diffIndex = Math.Max(0, Difficulties.IndexOf(SelectedDifficulty));
            StatusText = $"Launching Firefight: {SelectedFirefightMission.MapName} ({SelectedDifficulty})...";
            AppendLog($"Deploying Firefight on map {SelectedFirefightMission.MapName} (MapId: {SelectedFirefightMission.MapId}, Difficulty: {SelectedDifficulty})...");

            _modManager.SaveModState(Mods);
            _configService.Save(Config);

            bool launched = _processSupervisor.Launch(Config, Installation, dedicated: false,
                customMap: SelectedFirefightMission.MapName,
                customMode: "firefight",
                customDifficulty: diffIndex);

            if (!launched)
            {
                IsBusy = false;
                StatusText = "Firefight launch failed. Check log.";
            }
            else
            {
                StatusText = $"noble-client.exe running ({SelectedFirefightMission.MapName} Firefight).";
            }
        }

        private void LaunchCampaign()
        {
            IsBusy = true;
            int diffIndex = Math.Max(0, Difficulties.IndexOf(SelectedDifficulty));
            StatusText = $"Launching Campaign: {SelectedCampaignMission} ({SelectedDifficulty})...";
            AppendLog($"Deploying Campaign mission '{SelectedCampaignMission}' on {SelectedDifficulty} difficulty...");

            _modManager.SaveModState(Mods);
            _configService.Save(Config);

            bool launched = _processSupervisor.Launch(Config, Installation, dedicated: false,
                customMap: SelectedCampaignMission,
                customMode: "campaign",
                customDifficulty: diffIndex);

            if (!launched)
            {
                IsBusy = false;
                StatusText = "Campaign launch failed. Check log.";
            }
            else
            {
                StatusText = $"noble-client.exe running ({SelectedCampaignMission}).";
            }
        }

        private void LaunchClient()
        {
            LaunchMainMenu();
        }

        private void LaunchDedicated()
        {
            IsBusy = true;
            StatusText = "Starting noble-dedicated server...";
            AppendLog("Starting headless dedicated server instance...");

            _modManager.SaveModState(Mods);
            _configService.Save(Config);

            bool launched = _processSupervisor.Launch(Config, Installation, dedicated: true);
            if (!launched)
            {
                IsBusy = false;
                StatusText = "Dedicated server launch failed.";
            }
            else
            {
                StatusText = "Dedicated server running on port " + Config.DedicatedPort;
            }
        }

        private void BrowseMccPath()
        {
            var dialog = new OpenFolderDialog
            {
                Title = "Select Halo: The Master Chief Collection Root Directory",
                Multiselect = false
            };

            if (dialog.ShowDialog() == true)
            {
                Config.CustomMccPath = dialog.FolderName;
                OnPropertyChanged(nameof(Config));
                RunDiscovery();
                RefreshMods();
            }
        }

        private void SaveSettings()
        {
            _configService.Save(Config);
            _modManager.SaveModState(Mods);
            AppendLog("Configuration settings and mod load orders saved.");
            StatusText = "Settings saved successfully.";
        }

        private void AppendLog(string message)
        {
            LogOutput += $"[{DateTime.Now:HH:mm:ss}] {message}" + Environment.NewLine;
        }

        protected void OnPropertyChanged([CallerMemberName] string? name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }

    public class RelayCommand : ICommand
    {
        private readonly Action<object?> _execute;
        private readonly Predicate<object?>? _canExecute;

        public RelayCommand(Action<object?> execute, Predicate<object?>? canExecute = null)
        {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public bool CanExecute(object? parameter) => _canExecute == null || _canExecute(parameter);
        public void Execute(object? parameter) => _execute(parameter);
        public event EventHandler? CanExecuteChanged
        {
            add => CommandManager.RequerySuggested += value;
            remove => CommandManager.RequerySuggested -= value;
        }
    }
}
