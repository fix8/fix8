. "$PSScriptRoot\version.ps1"

function ReplaceVersion(
    [string]$InFile = "input-file-to-replace",
	[string]$OutFile = "output-result-file"
)
{
	(gc $InFile).replace('${RELEASE_MAJOR}', $RELEASE_MAJOR).replace('${RELEASE_MINOR}', $RELEASE_MINOR).replace('${RELEASE_DATE}', $RELEASE_DATE).replace('${RELEASE_REVISION}', $RELEASE_REVISION) > $OutFile
}
