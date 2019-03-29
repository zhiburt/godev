$ErrorActionPreference="Stop"
$BaseDir=$PSScriptRoot
$Password="password"
$Port=7699
$Python="python.exe"
$TestArgs=$args

$ErrorBoltKitNotAvailable=11
$ErrorCompilationFailed=12
$ErrorServerCleanUpFailed=18
$ErrorServerInstallFailed=13
$ErrorServerConfigFailed=14
$ErrorServerStartFailed=15
$ErrorServerStopFailed=16
$ErrorServerConfigurationError=17
$ErrorTestsFailed=199

trap {
    Exit 1
}

Function CheckBoltKit()
{
    Write-Host "Checking boltkit..."
    & $Python -c "import boltkit" *> $null
    If ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorBoltKitNotAvailable
            Message = "FATAL: The boltkit library is not available. Use 'pip install boltkit' to install."
        }
    }
}

Function Compile()
{
    Write-Host "Compiling..."
    & cmd.exe /c "$BaseDir\make_debug.cmd" $env:SEABOLT_TOOLCHAIN
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorCompilationFailed
            Message = "FATAL: Compilation failed."
        }
    }
}

Function Cleanup($Target)
{
    try
    {
        Get-ChildItem -Path $Target -ErrorAction Ignore | Remove-Item -Force -Recurse -ErrorAction Stop
    }
    catch
    {
        throw @{
            Code = $ErrorServerCleanUpFailed
            Message = "FATAL: Server directory cleanup failed."
        }
    }
}

Function InstallServer($Target, $Version)
{
    Write-Host "-- Installing server"
    $Server = Invoke-Expression "neoctrl-install $Version $Target"
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerInstallFailed
            Message = "FATAL: Server installation failed."
        }
    }
    Write-Host "-- Server installed at $Server"

    Write-Host "-- Configuring server to listen on port $Port"
    & neoctrl-configure "$Server" dbms.connector.bolt.listen_address=:$Port
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerConfigFailed
            Message = "FATAL: Unable to configure server port."
        }
    }

    Write-Host "-- Configuring server to accept IPv6 connections"
    & neoctrl-configure "$Server" dbms.connectors.default_listen_address=::
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerConfigFailed
            Message = "FATAL: Unable to configure server for IPv6."
        }
    }

    Write-Host "-- Setting initial password"
    & neoctrl-set-initial-password "$Password" "$Server"
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerConfigFailed
            Message = "FATAL: Unable to set initial password."
        }
    }

    Return $Server
}


Function StartServer($Server)
{
    Write-Host "-- Starting server"
    $BoltUri = Invoke-Expression "neoctrl-start $Server" | Select-String "^bolt:"
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerStartFailed
            Message = "FATAL: Failed to start server."
        }
    }
    Write-Host "-- Server is listening at $BoltUri"

    Return $BoltUri
}

Function StopServer($Server)
{
    Write-Host "-- Stopping server"
    & neoctrl-stop $Server
    if ( $LASTEXITCODE -ne 0 )
    {
        throw @{
            Code = $ErrorServerStopFailed
            Message = "FATAL: Failed to stop server."
        }
    }
}

Function RunTests($Version)
{
    $CompilationBase = "$Basedir\build-$( $env:SEABOLT_TOOLCHAIN )-debug"
    $SeaboltCli =  Get-ChildItem -Path $CompilationBase\bin -Filter seabolt-cli.exe -Recurse
    $SeaboltTest =  Get-ChildItem -Path $CompilationBase\bin -Filter seabolt-test.exe -Recurse

    $ServerBase = "$CompilationBase\server"
    Cleanup $ServerBase

    Write-Host "Testing against Neo4j $Version"
    $Server = InstallServer $ServerBase $Version

    $BoltUri = StartServer $Server
    try
    {
        Write-Host "-- Checking server"
        $env:BOLT_PASSWORD=$Password
        $env:BOLT_PORT=$Port
        & $SeaboltCli.FullName debug "UNWIND range(1, 10000) AS n RETURN n"
        if ( $LASTEXITCODE -ne 0 )
        {
            throw @{
                Code = $ErrorServerConfigurationError
                Message = "FATAL: Server is incorrectly configured."
            }
        }

        Write-Host "-- Running tests"
        & $SeaboltTest.FullName $TestArgs
        if ( $LASTEXITCODE -ne 0 )
        {
            throw @{
                Code = $ErrorTestsFailed
                Message = "FATAL: Test execution failed."
            }
        }
    }
    finally
    {
        StopServer $Server
    }
}


try
{
    $env:NEO4J_CHILD_SCRIPT="1"
    CheckBoltKit
    Compile

    $Neo4jVersion = "-e 3.4"
    If (Test-Path "env:NEOCTRLARGS") {
        $Neo4jVersion = $env:NEOCTRLARGS
    }

    RunTests $Neo4jVersion
}
catch
{
    $ErrorCode = 1
    $ErrorMessage = $_.Exception.Message

    If ( $_.TargetObject.Code -and $_.TargetObject.Message )
    {
        $ErrorCode = $_.TargetObject.Code
        $ErrorMessage = $_.TargetObject.Message
    }

    If ( $env:TEAMCITY_PROJECT_NAME )
    {
        $CleanedErrorMessage = $ErrorMessage -replace "[^a-zA-Z0-9., ]"

        Write-Host "##teamcity[buildProblem description='$($CleanedErrorMessage)' identity='$($ErrorCode)']"
        Write-Host "##teamcity[buildStatus status='FAILURE' text='$($CleanedErrorMessage)']"
    }
    Else
    {
        Write-Host "$($ErrorMessage) [$($ErrorCode)]"
    }

    Exit $ErrorCode
}
