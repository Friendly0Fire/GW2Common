Push-Location "$PSScriptRoot/../.."

git fetch --filter=tree:0 origin +refs/tags/*:refs/tags/*

$dirty = $false
$uncommittedChanges = (git status -s).Length -gt 0
$latestTag = git describe --tags --abbrev=0
if($LASTEXITCODE -ne 0) {
    $latestTag = "0.0.0"
    $dirty = $true
} else {
    $tagHash = git rev-list -n 1 "$latestTag"
    $currentHash = git rev-parse HEAD
    $dirty = $tagHash -ne $currentHash
    $dirty = $dirty -or $uncommittedChanges
}

$gitVer = $latestTag.Replace('.', ',').Substring(0, $latestTag.IndexOf('-')).Substring(1)
$fixVer = "9999"
if($latestTag.Contains("-pre"))
{
    $fixVer = $latestTag.Substring($latestTag.IndexOf("-pre") + 4)
}
$gitVer += "," + ($dirty ? $fixVer : "0")

$dirtySuffix = ""
if($dirty)
{
    $dirtySuffix = " (" + (git rev-parse --short HEAD)
    if($uncommittedChanges)
    {
        $dirtySuffix += "+"
    }
    $dirtySuffix += ")"
}

$gitVerStr = $latestTag.Substring(1) + $dirtySuffix

Write-Output "#define GIT_VER $gitVer
#define GIT_VER_STR ""$gitVerStr\0""" > include/Version.h