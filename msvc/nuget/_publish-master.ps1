param (
    [string]$ApiKey = "put-your-key-here"
)

$ver=(gc ..\..\version.sh)
$RELEASE_MAJOR=[regex]::Match($ver, "MAJOR_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_MINOR=[regex]::Match($ver, "MINOR_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_PATCH=[regex]::Match($ver, "PATCH_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
$VERSION="$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_PATCH"

$API_KEY = $ApiKey
echo $API_KEY
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.tests.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.stocklib.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
