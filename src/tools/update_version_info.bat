@set TARGET_DIR=%1
@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 2})-join\"`n\");&$s" %*&goto:eof

$target_dir = $env:TARGET_DIR
$version_file = $target_dir + "\.version.txt"
$header_file = $target_dir + "\.version.h"
$props_file = $target_dir + "\.version.props"

$commit_info_stopper = $False
filter commit-info{
    if (!$commit_info_stopper){
        $commit = $Null
        $v1 = $Null
        $v2 = $Null
        $v3 = $Null
        if ($_ -match '^[0-9a-z]+'){
            $commit = $Matches.0
        }
        if ($_ -match '(tag: v)([0-9]+)\.([0-9]+)\.([0-9]+)'){
            $v1 = $Matches.2
            $v2 = $Matches.3
            $v3 = $Matches.4
            $commit_info_stopper = $True
        }

        $result = New-Object PSObject | Select-Object commit, v1, v2, v3
        $result.commit = $commit
        $result.v1 = $v1
        $result.v2 = $v2
        $result.v3 = $v3
        return $result
    }
}

$commits = git log --pretty='%h%d' | commit-info
$v1 = $commits[$commits.count - 1].v1
$v2 = $commits[$commits.count - 1].v2
$v3 = $commits[$commits.count - 1].v3
$v4 = $commits.count - 1
if ($v4 -lt 0){
    $v4 = 0
}
$v4str =""
if ($v4 -gt 1){
    $v4str = ".{0}" -f $v4
}
$commit = $commits[0].commit
$suffix = ''
$suffix_product = "Commit: "
$file_mode = "0x0L"
if ((git status --porcelain).count -gt 0){
    $suffix = "+"
    $suffix_product = "modified from the commit:"
    $file_mode = "0x4L"
}

$ver_file = "{0},{1},{2},{3}" -f $v1, $v2, $v3, $v4
$ver_product = "{0},{1},{2},{3}" -f $v1, $v2, $v3, $v4
$ver_file_str = "{0}.{1}.{2}.{3}{4}" -f $v1, $v2, $v3, $v4, $suffix
$ver_product_str = "{0}.{1}.{2} [{3}{4}]" -f $v1, $v2, $v3, $suffix_product, $commit
$ver_title_str = "{0}.{1}.{2}{3}{4}" -f $v1, $v2, $v3, $v4str, $suffix

if ($target_dir.Length -le 0){
    '{0}' -f $ver_title_str
    return
}

if (Test-Path $version_file){
    $saved_ver = Get-Content $version_file
    if ($saved_ver.count -gt 0 -and $saved_ver -eq $ver_file_str){
        "version files are not updated"
        return
    }
}

$year = Get-Date -Format "yyyy"
$copyright = "2021-{0}" -f $year

$ver_file_str > $version_file

"#pragma once" > $header_file
"#define VER_FILE_VERSION {0}" -f $ver_file >> $header_file
"#define VER_PRODUCT_VERSION {0}" -f $ver_product >> $header_file
'#define VERSTR_FILE_VERSION "{0}"' -f $ver_file_str >> $header_file
'#define VERSTR_PRODUCT_VERSION "{0}"' -f $ver_product_str >> $header_file
'#define VERSTR_TITLE_VERSION "{0}"' -f $ver_title_str >> $header_file
"#define VER_FILE_MODE {0}" -f $file_mode >> $header_file
'#define COPYRIGHT_STR "{0}"' -f $copyright >> $header_file
'#define COMMIT_STR "{0}"' -f $commit >> $header_file
'#define HOOKLIBNAME fsmapperhook_{0}.lib' -f $ver_title_str >> $header_file
'#define HOOKDLLNAME_STR "fsmapperhook_{0}.dll"' -f $ver_title_str >> $header_file

'<Project>' > $props_file
'  <PropertyGroup>' >> $props_file
'    <FsmapperVersion>{0}</FsmapperVersion>' -f $ver_title_str >> $props_file
'  </PropertyGroup>' >> $props_file
'</Project>' >> $props_file

"version files have been updated"
