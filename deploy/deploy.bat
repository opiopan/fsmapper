@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 1})-join\"`n\");&$s" %*&goto:eof

$src = "..\src\x64\Release"
$assets = $src + "\Assets"
$exe = $src + "\fsmapper.exe"
$dll = $src + "\*.dll"
$xbf = $src + "\*.xbf"
$samples = "..\samples"

$dest = "fsmapper"
$package = "fsmapper.zip"

if (Test-Path $dest){
    Remove-Item $dest -Recurse
}
if (Test-Path $package){
    Remove-Item $package
}

New-Item $dest -ItemType Directory
Copy-Item $assets $dest -Recurse
Copy-Item $samples $dest -Recurse
Copy-Item $exe $dest
Copy-Item $dll $dest
Copy-Item $xbf $dest

Compress-Archive -Path $dest -DestinationPath $package