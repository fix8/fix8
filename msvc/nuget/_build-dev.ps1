$ver=(gc ..\..\version.sh)
$RELEASE_MAJOR=[regex]::Match($ver, "MAJOR_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_MINOR=[regex]::Match($ver, "MINOR_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_PATCH=[regex]::Match($ver, "PATCH_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
$VERSION="$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_PATCH"
(gc fix8.dev.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION).replace('${VERSION}', $VERSION) > _1.autopkg
Write-NuGetPackage _1.autopkg -NoSplit
(gc fix8.dev.tests.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION).replace('${VERSION}', $VERSION) > _2.autopkg
Write-NuGetPackage _2.autopkg -NoSplit
(gc fix8.dev.stocklib.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION).replace('${VERSION}', $VERSION) > _3.autopkg
Write-NuGetPackage _3.autopkg -NoSplit
rm _1.autopkg
rm _2.autopkg
rm _3.autopkg
