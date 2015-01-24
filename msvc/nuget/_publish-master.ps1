param (
    [string]$ApiKey = "put-your-key-here"
)

$RELEASE_MAJOR = 1
$RELEASE_MINOR = 3
$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
$API_KEY = $ApiKey
echo $API_KEY
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.tests.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.tests.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.stocklib.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.stable.stocklib.symbols.$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION.nupkg
