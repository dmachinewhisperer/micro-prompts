$arduino_lib_path = "C:\Users\asogw\Documents\Arduino\libraries\"
$folderName = "micro-prompt-ino"
$destinationFolder = Join-Path -Path $arduino_lib_path -ChildPath $folderName

if (-not (Test-Path $destinationFolder)) {
    mkdir $destinationFolder
} else {
    Write-Output "Folder already exists: $destinationFolder, contents will be overwritten"
}

$sourceFolder = "."

$fileList = @{
    "ino/examples"         = "examples";
    "ino/src"              = "src";
    "ino/keywords.txt"     = ".";
    "ino/library.properties" = ".";
    "ino/README.md"        = ".";
    "src/common"           = "src/common";
    "LICENSE"              = "."
}

foreach ($item in $fileList.Keys) {
    $sourcePath = Join-Path -Path $sourceFolder -ChildPath $item
    $subDir = $fileList[$item]
    $destinationPath = Join-Path -Path $destinationFolder -ChildPath $subDir

    if (-not (Test-Path $destinationPath)) {
        New-Item -Path $destinationPath -ItemType Directory -Force | Out-Null
    }

    if (Test-Path $sourcePath -PathType Leaf) {
        #file copy
        Write-Host "Copying file: $sourcePath to $destinationPath"
        Copy-Item -Path $sourcePath -Destination $destinationPath -Force
    }
    elseif (Test-Path $sourcePath -PathType Container) {
        #folder copy
        Write-Host "Copying folder: $sourcePath to $destinationPath"
        Copy-Item -Path (Join-Path $sourcePath '*') -Destination $destinationPath -Recurse -Force
    }
    else {
        Write-Host "Item not found: $sourcePath"
    }
}

Write-Host "Copy operation complete."
