$rootDir = $PSScriptRoot

$keysToDelete = @(
    "obj"
    "build"
    ".sln"
    ".user"
    ".vcxproj"
    ".filters"
    ".ini"
    ".slnx"
)

if (-not $keysToDelete -or $keysToDelete.Count -eq 0) {
    Write-Host "No keys to delete"
    return
}

foreach ($key in ($keysToDelete | Select-Object -Unique)) {
    if ($key -match "^\.") {
        $filesToDelete = Get-ChildItem -Path $rootDir -Recurse -File -Force |
            Where-Object { $_.Extension -ieq $key }

        if ($filesToDelete) {
            foreach ($file in $filesToDelete) {
                try {
                    Write-Host "Deleting file: $($file.FullName)"
                    Remove-Item -Path $file.FullName -Force -ErrorAction Stop
                }
                catch {
                    Write-Host "Error deleting file: $($file.FullName) - $($_.Exception.Message)"
                }
            }
        }
        else {
            Write-Host "No files found with extension: $key"
        }
    }
    else {
        $dirsToDelete = Get-ChildItem -Path $rootDir -Recurse -Directory -Force |
            Where-Object { $_.Name -ieq $key }

        if ($dirsToDelete) {
            foreach ($dir in $dirsToDelete) {
                try {
                    Write-Host "Deleting directory: $($dir.FullName)"
                    Remove-Item -Path $dir.FullName -Recurse -Force -ErrorAction Stop
                }
                catch {
                    Write-Host "Error deleting directory: $($dir.FullName) - $($_.Exception.Message)"
                }
            }
        }
        else {
            Write-Host "No directory found with name: $key"
        }
    }
}