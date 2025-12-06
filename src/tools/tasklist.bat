@SET CURRENT_DIR=%~dp0
@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 2})-join\"`n\");&$s" %*&goto:eof

$current_dir = $env:CURRENT_DIR
$version = Get-Content -Path "$($current_dir)..\.version.txt"
tasklist /m "fsmapperhook_$($version).dll"
