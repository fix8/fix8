$RELEASE_MAJOR = 1
$RELEASE_MINOR = 3
$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
(gc fix8.stable.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION) > _1.autopkg
Write-NuGetPackage _1.autopkg
(gc fix8.stable.tests.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION) > _2.autopkg
Write-NuGetPackage _2.autopkg
(gc fix8.stable.stocklib.autopkg).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION) > _3.autopkg
Write-NuGetPackage _3.autopkg
rm _1.autopkg
rm _2.autopkg
rm _3.autopkg
