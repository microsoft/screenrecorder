Write-Output "Building python project."

# Get the path of the script
$scriptPath = $PSScriptRoot

# Construct the path to the spec file
$specFilePath = Join-Path -Path $scriptPath -ChildPath "LaunchDetectorScript.spec"

pyinstaller $specFilePath
