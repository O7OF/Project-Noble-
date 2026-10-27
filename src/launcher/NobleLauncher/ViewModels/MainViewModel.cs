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

        public ObservableCollection<ModPackage> Mods { get; } = new();

        public ICommand LaunchGameCommand { get; }
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

            LaunchGameCommand = new RelayCommand(_ => LaunchClient(), _ => !IsBusy && Installation.IsValid);
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

        private void LaunchClient()
        {
            IsBusy = true;
            StatusText = "Launching Halo Reach Community Edition...";
            AppendLog("Initiating client launch sequence...");

            _modManager.SaveModState(Mods);
            _configService.Save(Config);

            bool launched = _processSupervisor.Launch(Config, Installation, dedicated: false);
            if (!launched)
            {
                IsBusy = false;
                StatusText = "Launch failed. Check log for details.";
            }
            else
            {
                StatusText = "noble-client.exe is running.";
            }
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
