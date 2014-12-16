param (
    [string]$ApiKey = "put-your-key-here"
)

. "$PSScriptRoot\scripts\version.ps1"

$API_KEY = $ApiKey
echo $API_KEY
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.tests.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.tests.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.stocklib.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.stocklib.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
