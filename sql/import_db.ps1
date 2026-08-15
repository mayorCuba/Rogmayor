<#
.SYNOPSIS
  Importa las bases de datos de LegionCore 7.3.5 (auth, characters, world, hotfixes)
  en el orden correcto, aplicando los updates y el contenido opcional.

.EXAMPLE
  .\sql\import_db.ps1 -MysqlUser root -MysqlPassword tu_clave

.EXAMPLE
  .\sql\import_db.ps1 -MysqlUser root -MysqlPassword tu_clave -MysqlPath "C:\mysql\bin\mysql.exe" -Patch72 disable -Patch73 skip
#>
param(
    [string]$MysqlUser = "root",
    [string]$MysqlPassword = "",
    [string]$MysqlHost = "127.0.0.1",
    [int]$MysqlPort = 3306,
    [string]$MysqlPath = "",
    [ValidateSet("enable", "disable", "skip")]
    [string]$Patch72 = "enable",
    [ValidateSet("enable", "disable", "skip")]
    [string]$Patch73 = "enable",
    [switch]$ContinueOnError,
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ScriptDir   = $PSScriptRoot
$BaseDir     = Join-Path $ScriptDir "base"
$UpdatesDir  = Join-Path $ScriptDir "updates"
$OptionalDir = Join-Path $ScriptDir "optional"
$LogFile     = Join-Path $ScriptDir "import_db.log"

$Failures = 0

function Write-Log {
    param([string]$Message)
    $stamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $line = "[$stamp] $Message"
    Write-Host $line
    Add-Content -LiteralPath $LogFile -Value $line -Encoding utf8
}

function Resolve-Mysql {
    if ($MysqlPath -ne "" -and (Test-Path -LiteralPath $MysqlPath)) {
        return $MysqlPath
    }
    $cmd = Get-Command mysql -ErrorAction SilentlyContinue
    if ($cmd) {
        return $cmd.Source
    }
    throw "No se encontro mysql.exe. Indicalo con -MysqlPath."
}

function Invoke-MysqlScript {
    param([string]$Db, [string]$File, [string]$Label)
    if (-not (Test-Path -LiteralPath $File)) {
        Write-Log "FALTA el archivo: $File"
        return $false
    }
    $env:MYSQL_PWD = $Script:MysqlPassword
    $cmd = '"' + $Script:MysqlBin + '" --max_allowed_packet=1G -h "' + $Script:MysqlHost + '" -P ' + $Script:MysqlPort +
           ' -u "' + $Script:MysqlUser + '" "' + $Db + '" < "' + $File + '"'
    Write-Log "Importando $Label -> $Db ($([math]::Round((Get-Item -LiteralPath $File).Length / 1MB, 1)) MB)"
    & cmd.exe /c $cmd 2>&1 | ForEach-Object { Write-Host $_ }
    $ok = ($LASTEXITCODE -eq 0)
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
    if (-not $ok) {
        Write-Log "ERROR importando $Label (exit code $LASTEXITCODE)"
        return $false
    }
    return $true
}

function Invoke-MysqlEval {
    param([string]$Sql, [string]$Label)
    $env:MYSQL_PWD = $Script:MysqlPassword
    $cmd = '"' + $Script:MysqlBin + '" -h "' + $Script:MysqlHost + '" -P ' + $Script:MysqlPort +
           ' -u "' + $Script:MysqlUser + '" -e "' + $Sql + '"'
    & cmd.exe /c $cmd 2>&1 | ForEach-Object { Write-Host $_ }
    $ok = ($LASTEXITCODE -eq 0)
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
    return $ok
}

function Invoke-MysqlScalar {
    param([string]$Sql)
    $env:MYSQL_PWD = $Script:MysqlPassword
    $cmd = '"' + $Script:MysqlBin + '" -N -h "' + $Script:MysqlHost + '" -P ' + $Script:MysqlPort +
           ' -u "' + $Script:MysqlUser + '" -e "' + $Sql + '"'
    $out = & cmd.exe /c $cmd 2>&1
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
    if ($LASTEXITCODE -ne 0) { return "" }
    return (($out | Select-Object -Last 1) -as [string]).Trim()
}

function Import-Lists {
    param([string]$Db, [string[]]$Files)
    foreach ($f in $Files) {
        $ok = Invoke-MysqlScript -Db $Db -File $f -Label (Split-Path $f -Leaf)
        if (-not $ok) {
            $Script:Failures++
            if (-not $ContinueOnError) { throw "Importacion abortada por error en: $f" }
        }
    }
}

try {
    Write-Log "=== Inicio de importacion LegionCore ==="
    Write-Log ("Opcional 7.2: {0} | Opcional 7.3: {1} | ContinueOnError: {2}" -f $Patch72, $Patch73, $ContinueOnError)

    $Script:MysqlBin = Resolve-Mysql
    Write-Log "mysql: $MysqlBin"
    if (-not (Invoke-MysqlEval -Sql "SELECT 1" -Label "test conexion")) {
        throw "No se pudo conectar a MySQL. Revisa usuario/clave/host/puerto."
    }

    foreach ($db in @("auth", "characters", "world", "hotfixes")) {
        if (-not (Invoke-MysqlEval -Sql "CREATE DATABASE IF NOT EXISTS $db CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci" -Label "crear base $db")) {
            throw "No se pudo crear la base $db"
        }
        if (-not $Force) {
            $count = Invoke-MysqlScalar -Sql "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='$db'"
            if ($count -match '^\d+$' -and [int]$count -gt 0) {
                throw "La base $db ya tiene $count tablas. Reimportarla desde cero borraria esos datos. Usa -Force para confirmar que quieres pisarla."
            }
        }
    }

    Write-Log "--- AUTH ---"
    Import-Lists -Db "auth" -Files @(
        (Join-Path $BaseDir "LegionCore_auth_2020_04_03.sql"),
        (Join-Path $UpdatesDir "auth\0001_merge_account_tables.sql"),
        (Join-Path $UpdatesDir "auth\0002_battlepay_cleanup.sql"),
        (Join-Path $UpdatesDir "auth\0003_battlepay_rework.sql")
    )

    Write-Log "--- CHARACTERS ---"
    Import-Lists -Db "characters" -Files @(
        (Join-Path $BaseDir "LegionCore_characters_2020_04_03.sql"),
        (Join-Path $UpdatesDir "characters\0001_default_enable_2fa_backpack_slots.sql")
    )

    Write-Log "--- WORLD (base + updates) ---"
    $worldZip = Join-Path $BaseDir "LegionCore_world_2020_04_25.zip"
    $worldSql = Join-Path $BaseDir "LegionCore_world_2020_04_25.sql"
    $worldTmp = ""
    if (-not (Test-Path -LiteralPath $worldSql)) {
        if (-not (Test-Path -LiteralPath $worldZip)) {
            throw "No existe ni el world .sql ni su .zip en $BaseDir"
        }
        Write-Log "Extrayendo $worldZip ..."
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $worldTmp = Join-Path $BaseDir "LegionCore_world_extracted_tmp.sql"
        [System.IO.Compression.ZipFile]::ExtractToFile($worldZip, $worldTmp)
        $worldSql = $worldTmp
    }
    try {
        Import-Lists -Db "world" -Files @($worldSql)
        Import-Lists -Db "world" -Files @(
            (Join-Path $UpdatesDir "world\0001_creature_text_broadcastid_linking.sql"),
            (Join-Path $UpdatesDir "world\0002_allied_races_achievement_unlock.sql"),
            (Join-Path $UpdatesDir "world\0003_battlepay_cleanup.sql"),
            (Join-Path $UpdatesDir "world\0004_demon_hunter_intro_questline_fixes.sql"),
            (Join-Path $UpdatesDir "world\0005_legion_intro_quest_fixes.sql"),
            (Join-Path $UpdatesDir "world\0006_wod_garrisons.sql"),
            (Join-Path $UpdatesDir "world\0007_misc.sql"),
            (Join-Path $UpdatesDir "world\0008_misc.sql"),
            (Join-Path $UpdatesDir "world\0009_wod_garrison_fixes.sql"),
            (Join-Path $UpdatesDir "world\0010_broken_shore_scenario.sql"),
            (Join-Path $UpdatesDir "world\0011_misc.sql"),
            (Join-Path $UpdatesDir "world\0012_misc.sql"),
            (Join-Path $UpdatesDir "world\0013_misc.sql"),
            (Join-Path $UpdatesDir "world\0014_battlepay_rework.sql"),
            (Join-Path $UpdatesDir "world\0015_battlepay_group_fixes.sql"),
            (Join-Path $UpdatesDir "world\0016_quest_autocomplete.sql"),
            (Join-Path $UpdatesDir "world\0017_misc.sql"),
            (Join-Path $UpdatesDir "world\0018_quest_drops_and_creature_corrections.sql")
        )
        Write-Log "--- WORLD (catalogo battlepay) ---"
        Import-Lists -Db "world" -Files @(
            (Join-Path $UpdatesDir "world\Battlepay\battlepay_display_info.sql")
        )
    }
    finally {
        if ($worldTmp -ne "" -and (Test-Path -LiteralPath $worldTmp)) {
            Remove-Item -LiteralPath $worldTmp -Force
        }
    }

    Write-Log "--- HOTFIXES ---"
    Import-Lists -Db "hotfixes" -Files @(
        (Join-Path $BaseDir "LegionCore_hotfixes_2020_04_03.sql"),
        (Join-Path $UpdatesDir "hotfix\0001_fix_garrison_mission_db_structure.sql"),
        (Join-Path $UpdatesDir "hotfix\0002_battlepay_rework.sql")
    )

    Write-Log "--- CONTENIDO OPCIONAL ---"
    if ($Patch72 -ne "skip") {
        Import-Lists -Db "world" -Files @(
            (Join-Path $OptionalDir ("{0}-patch-7.2-content.sql" -f $Patch72))
        )
    }
    if ($Patch73 -ne "skip") {
        Import-Lists -Db "world" -Files @(
            (Join-Path $OptionalDir ("{0}-patch-7.3-content.sql" -f $Patch73))
        )
    }

    if ($Failures -gt 0) {
        Write-Log "=== IMPORTACION TERMINADA CON $Failures ERROR(ES) (modo ContinueOnError) ==="
        exit 1
    }
    Write-Log "=== IMPORTACION COMPLETADA CON EXITO ==="
}
catch {
    Write-Log ("EXCEPCION: " + $_.Exception.Message)
    Write-Log "=== IMPORTACION FALLIDA ==="
    exit 1
}
