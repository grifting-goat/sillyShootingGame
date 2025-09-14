# Create Desktop Shortcut for Silly Shooting Game
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$Home\Desktop\Silly Shooting Game.lnk")
$Shortcut.TargetPath = "$PWD\SillyShootingGame.bat"
$Shortcut.WorkingDirectory = "$PWD"
$Shortcut.Description = "Enhanced AssaultCube with Knife Lunge System"
$Shortcut.Save()

Write-Host "Desktop shortcut created: 'Silly Shooting Game'" -ForegroundColor Green
