@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 1})-join\"`n\");&$s" %*&goto:eof

$src = "..\src\x64\Release"
$coresrc = "..\src\core"
$assets = $src + "\Assets"
$exe = $src + "\fsmapper.exe"
$dll = $src + "\*.dll"
$xbf = $src + "\*.xbf"
$samples = "..\samples"
$sdk_samples = "..\sdk_samples"

$dest = "fsmapper"
$dest_sdk = $dest + "\sdk"
$dest_sdkinc = $dest_sdk + "\include"
$dest_sdklib = $dest_sdk + "\lib"
$dest_sdksamples = $dest_sdk + "\samples"
$package = "fsmapper.zip"

if (Test-Path $dest){
    Remove-Item $dest -Recurse -Force
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

New-Item $dest_sdk -ItemType Directory
New-Item $dest_sdkinc -ItemType Directory
New-Item $dest_sdklib -ItemType Directory
Copy-Item "$($coresrc)\mapperplugin.h" $dest_sdkinc
Copy-Item "$($src)\fsmappercore.lib" $dest_sdklib
Copy-Item $sdk_samples $dest_sdk -Recurse
Rename-Item -Path "$($dest_sdk)\sdk_samples" -NewName "samples"

Rename-Item -Path "$($dest)\Assets" -NewName "assets.tmp"
Rename-Item -Path "$($dest)\assets.tmp" -NewName "assets"

Compress-Archive -Path $dest -DestinationPath $package
