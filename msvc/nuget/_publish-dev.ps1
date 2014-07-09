param (
    [string]$ApiKey = "put-your-key-here"
)

$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
$API_KEY = $ApiKey
echo $API_KEY
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.1.2.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.symbols.1.2.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.tests.1.2.$RELEASE_DATE.$RELEASE_REVISION.nupkg
Publish-NuGetPackage -ApiKey $API_KEY fix8.dev.tests.symbols.1.2.$RELEASE_DATE.$RELEASE_REVISION.nupkg
