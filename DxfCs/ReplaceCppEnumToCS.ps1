$enumDict = @{}
$startEnum = $false
$enumName = ""
$enumArray = $null
$prefixLen = "public enum ".Length
$enumFileLines = Get-content "G:\p4ws\ClassLibrary1\ACADConst.cs"
$enumFileLines | ForEach-Object {
    if ($_.StartsWith("public enum ")) {
        $startEnum = $true;
        $enumName = $_.SubString($prefixLen).Trim();
        $enumArray = New-Object System.Collections.ArrayList
        $enumDict.Add($enumName, $enumArray)
    } elseif ($startEnum) {
        if ($_.StartsWith("};")) {
            $startEnum = $false;
        } elseif ($_.StartsWith("{")) {
            # ignore it.
        }
        else {
            $enumArray.Add($_.Split("=")[0].Trim()) | Out-Null
        }
    }
}

$enumFileLines = Get-content "G:\p4ws\ClassLibrary1\DxfWriter.cs"
$enumFileLines | ForEach-Object {
    :loop foreach($enum in $enumDict.Keys) {
        foreach($enumVal in $enumDict[$enum]) {
            if ($_ -match $enumVal) {
                $_ = $_ -replace "\b$enumVal\b", "$enum.$enumVal"
                break loop
            }
        }
    }
    $_
} | Set-Content "G:\p4ws\ClassLibrary1\DxfWriter2.cs"


