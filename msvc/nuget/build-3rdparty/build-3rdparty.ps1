$ver=(get-content ..\..\..\version.sh)
$RELEASE_MAJOR=[regex]::Match($ver, "MAJOR_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_MINOR=[regex]::Match($ver, "MINOR_VERSION_NUM=(\d+)").Groups[1]
#$RELEASE_PATCH=[regex]::Match($ver, "PATCH_VERSION_NUM=(\d+)").Groups[1]
$RELEASE_DATE = get-date -uformat "%Y%m%d"
$RELEASE_REVISION = 1
#$VERSION="$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_PATCH"
$VERSION="$RELEASE_MAJOR.$RELEASE_MINOR.$RELEASE_DATE.$RELEASE_REVISION"

$BUILD_ROOT=get-location
foreach($v in ("v142")) {
    set-location $BUILD_ROOT
    new-item $v -itemtype directory -force | out-null
    & xcopy /y /s /d /q /h $BUILD_ROOT\vcpkg\* $v
    set-location $BUILD_ROOT\$v
    if (!(test-path "vcpkg.exe")) {
        start-process -filepath "$env:comspec" -argumentlist "/c bootstrap-vcpkg.bat" -wait -nonewwindow
    }
    foreach($t in ("x64-windows", "x86-windows")) {
        foreach($l in ("openssl", "zlib", "getopt", "poco", "tbb", "gtest")) {
            $pkg_list="$pkg_list ${l}:$t"
        }
        $all_pkg_list="$all_pkg_list $pkg_list"
        start-process -filepath .\vcpkg.exe -argumentlist "install",$pkg_list -wait -nonewwindow
    }
    start-process -filepath .\vcpkg.exe -argumentlist "export","--nuget","--nuget-id=fix8.deps","--nuget-version=$VERSION",$all_pkg_list -wait -nonewwindow
    copy-item *.nupkg -destination ..
    set-location $BUILD_ROOT
}
set-location $BUILD_ROOT
