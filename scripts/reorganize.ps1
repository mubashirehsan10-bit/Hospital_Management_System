# Reorganize project files into src/ and include/ and put GUI + helpers into separate folders.
# Run from repository root (PowerShell 5+). This script will:
# - create directories (include, src, src\gui, src\helpers, data, assets if missing)
# - move .h files -> include\
# - move .cpp files -> src\ (some to src\gui and src\helpers per lists)
# - attempt to use git mv if .git exists (keeps history); otherwise Move-Item
# - update .vcxproj and .vcxproj.filters Include paths (backups created)
# Always inspect changes before commit.
set -e

# CONFIG: list special placements
$guiCpp = @("MediCoreGui.cpp","MediCoreGuiAPI.cpp")       # main GUI / GUI-related implementation
$helperCpp = @("utility.cpp","FileHandler.cpp","Validator.cpp","HospitalException.cpp","SlotUnavailableException.cpp","InsufficientFundsException.cpp") # helpers
$dataFiles = @("patients.txt","doctors.txt","appointments.txt","bills.txt","prescriptions.txt","admin.txt","discharged.txt","security_log.txt")

# Create folders
$folders = @("src","src\gui","src\helpers","include","data","assets","docs","scripts")
foreach ($f in $folders) { if (-not (Test-Path $f)) { New-Item -ItemType Directory -Path $f | Out-Null } }

$useGit = Test-Path ".git"

function MoveFileGitAware($srcPath, $dstPath) {
    if (-not (Test-Path $srcPath)) { return }
    if ($useGit) {
        & git mv -f -- "$srcPath" "$dstPath" 2>$null
        if ($LASTEXITCODE -ne 0) {
            # fallback to Move-Item if git mv failed (e.g., file already moved)
            Move-Item -Force -Path $srcPath -Destination $dstPath
        }
    } else {
        Move-Item -Force -Path $srcPath -Destination $dstPath
    }
}

Write-Host "Moving header files to include/ ..."
Get-ChildItem -Path . -Filter *.h -File -Recurse | ForEach-Object {
    $relative = $_.FullName.Substring((Get-Location).Path.Length+1) -replace '\\','\'
    $dst = "include\" + $_.Name
    Write-Host "  $relative -> $dst"
    MoveFileGitAware $_.FullName $dst
}

Write-Host "Moving .cpp files..."
# Move GUI .cpp
foreach ($name in $guiCpp) {
    if (Test-Path $name) {
        Write-Host "  GUI: $name -> src\gui\$name"
        MoveFileGitAware $name ("src\gui\" + $name)
    }
}
# Move helper .cpp
foreach ($name in $helperCpp) {
    if (Test-Path $name) {
        Write-Host "  Helper: $name -> src\helpers\$name"
        MoveFileGitAware $name ("src\helpers\" + $name)
    }
}
# Move remaining .cpp to src/
Get-ChildItem -Path . -Filter *.cpp -File | ForEach-Object {
    if ($guiCpp -contains $_.Name -or $helperCpp -contains $_.Name) { return }
    $relative = $_.FullName.Substring((Get-Location).Path.Length+1) -replace '\\','\'
    $dst = "src\" + $_.Name
    Write-Host "  $relative -> $dst"
    MoveFileGitAware $_.FullName $dst
}

Write-Host "Moving data files to data/ ..."
foreach ($f in $dataFiles) {
    if (Test-Path $f) {
        Write-Host "  $f -> data\$f"
        MoveFileGitAware $f ("data\" + $f)
    }
}

# Leave assets/ in repo root (if present). If assets/ is nested, you can move it manually.

# Backup project files and update include paths inside .vcxproj and .vcxproj.filters
$projFiles = Get-ChildItem -Filter *.vcxproj -File
$filterFiles = Get-ChildItem -Filter *.vcxproj.filters -File

function BackupAndReplace($file) {
    $bak = $file.FullName + ".bak"
    Copy-Item -Force $file.FullName $bak
    Write-Host "Backed up $($file.Name) -> $($file.Name).bak"

    $text = Get-Content -Raw -LiteralPath $file.FullName

    # Replace occurrences of old simple filenames with new relative paths
    # (ClCompile / ClInclude / None entries use Include="path\file.ext")
    $replacements = @{}

    # headers moved to include\
    Get-ChildItem -Path include -Filter *.h -File | ForEach-Object {
        $old = $_.Name
        $new = "include\" + $_.Name
        $replacements[$old] = $new
    }
    # cpp moved to src\, src\gui\, src\helpers
    Get-ChildItem -Path src -Recurse -Filter *.cpp -File | ForEach-Object {
        $old = $_.Name
        $rel = $_.FullName.Substring((Get-Location).Path.Length+1) -replace '\\','\'
        $new = $rel
        $replacements[$old] = $new
    }
    # data and assets to data\ or assets\
    Get-ChildItem -Path data -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
        $old = $_.Name
        $rel = $_.FullName.Substring((Get-Location).Path.Length+1) -replace '\\','\'
        $replacements[$old] = $rel
    }
    if (Test-Path "assets") {
        Get-ChildItem -Path assets -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
            $old = $_.Name
            $rel = $_.FullName.Substring((Get-Location).Path.Length+1) -replace '\\','\'
            $replacements[$old] = $rel
        }
    }

    # Apply replacements (simple replace on Include="...filename")
    foreach ($kv in $replacements.GetEnumerator()) {
        $oldName = [Regex]::Escape($kv.Key)
        $newRel = $kv.Value -replace '\\','\\'
        # replace occurrences where the old filename appears inside an Include attribute value
        $pattern = "Include\s*=\s*`"([^`"]*\\)?" + $oldName + "`""
        $text = [Regex]::Replace($text, $pattern, "Include=`"$newRel`"", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
    }

    Set-Content -LiteralPath $file.FullName -Value $text -Encoding UTF8
    Write-Host "Updated paths in $($file.Name)"
}

foreach ($p in $projFiles) { BackupAndReplace $p }
foreach ($f in $filterFiles) { BackupAndReplace $f }

Write-Host ""
Write-Host "Done. Please open the solution in Visual Studio and allow it to reload changed files."
Write-Host "Next manual steps recommended:"
Write-Host "  1) Open __Project Properties__ > __C/C++__ > __Additional Include Directories__ and add: $(ProjectDir)include"
Write-Host "  2) Verify linker paths for SFML libs in __Linker__ > __General__ and __Linker__ > __Input__"
Write-Host "  3) Inspect .vcxproj.filters (backup created) to adjust filters if you want different grouping in Solution Explorer."
Write-Host "  4) Run: git status to review moved files, then commit the changes."

# Visual Studio
.vs/
*.user
*.suo
*.VC.db
ipch/
Debug/
Release/
build/
bin/
obj/

# Visual Studio Code
.vscode/

# OS
Thumbs.db
.DS_Store

# CMake / generated
CMakeFiles/
CMakeCache.txt
Makefile

# Data (if you want these untracked, otherwise remove)
data/*.txt

# SFML libs (if you added them)
libs/