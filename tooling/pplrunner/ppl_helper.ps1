param(
    [Parameter(Mandatory=$true)]
    [string]$ExecutableToRun,
    [string]$CommandLineArgs = "",
    [string]$PfxFile = "ppl_runner.pfx",
    [string]$PfxPassw = "password"
)

# Check if running as admin
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) { 
    Write-Warning "You do not have Administrator rights to run this script!`nPlease re-run this script as an Administrator!"
    exit 1
}

# Helper function to confirm actions before taking them
function Confirm-Action() {
    # Define an input paramter for the prompt
    param(
        [Parameter(Mandatory=$true)]
        [string]$prompt
    )
    $answer = (Read-Host "$prompt (y/n)").ToLower()
    if ($answer -eq "y" -or $answer -eq "yes") {
        return $true
    }
    return $false
}

# Check if test signing is enabled
$bcdOutput = $(bcdedit /enum ACTIVE)
$testSigningLine = $bcdOutput | Select-String -Pattern "^testsigning\s*(Yes|No)\s*$"
$isTestSigningEnabled = $(($testSigningLine -replace ".*testsigning\s*(.*)", '$1').ToLower() -eq "yes")
if (-not $isTestSigningEnabled) {
    Write-Warning "Test signing is not enabled!"
    if (Confirm-Action "Would you like to enable test signing?") {
        bcdedit /set testsigning on
        Write-Host "Test signing enabled!"
    } else {
        Write-Host "Test signing not enabled!"
        exit 1
    }
    if (Confirm-Action "Would you like to reboot now to apply the change?") {
        Write-Host "Rebooting now..."
        Restart-Computer
    } else {
        Write-Host "Reboot to apply the change!"
        exit 1
    }
    exit 1
}


Write-Host "Installing ppl_runner service..."
$pplPath = Resolve-Path ppl_runner.exe
&"$pplPath" install

$fullPathToPfx = Resolve-Path $PfxFile
Write-Host "Full path to certificate: $fullPathToPfx"

$isRootCertInstalled = $null -ne (Get-ChildItem -Path Cert:\LocalMachine\Root | Where-Object -Property Subject -Like 'CN=ppl_runner')
if (-not $isRootCertInstalled) {
    $shouldAddRoot = (Confirm-Action @"
    Would you like to add the ppl_runner root certificate to the trusted root store?
    This will allow anyone with access to this git repo to run arbitrary code as a trusted process.
    If not added, code signing may silently fail.
"@
    )
    if ($shouldAddRoot) {
        Write-Host "Adding $PfxFile to trusted root store..."
        Import-PfxCertificate $fullPathToPfx -Password (ConvertTo-SecureString "$PfxPassw" -AsPlainText -Force) 'Cert:\LocalMachine\Root'
    }
}

Write-Host "Signing $ExecutableToRun with $pfxFile..."

$fullPathToExecutable = Resolve-Path $ExecutableToRun
Write-Host "Full path to executable: $fullPathToExecutable"

# The system I ran this on doesn't have the powershell version where you can pass the password to Get-PfxCertificate, so we directly get it.
$cert = New-Object System.Security.Cryptography.X509Certificates.X509Certificate
$cert.Import($fullPathToPfx, $PfxPassw, [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::DefaultKeySet)

# Sign the executable
Set-AuthenticodeSignature -Certificate $cert -HashAlgorithm SHA256 -FilePath $fullPathToExecutable

$PPLRUNNER_PATH = 'HKLM:\SOFTWARE\PPL_RUNNER'


# Check if the value is not null, confirm overwrite
$currentValue = Get-ItemProperty -Path $PPLRUNNER_PATH | Select-Object -Property '(default)'
if ($null -ne $currentValue) {
    if (-not (Confirm-Action "Would you like to overwrite the existing value of `n`t$currentValue`n")) {
        Write-Host "Not overwriting existing value!"
        exit 1
    }
}

# Set the value
Set-ItemProperty -Path $PPLRUNNER_PATH -Name '(default)' -Value "$fullPathToExecutable $ExecutableCommandLine"

Write-Host "Starting the PPL service."
Start-Service -Name 'ppl_runner' | Out-Null

$serviceExitCode = Get-WmiObject Win32_Service -Filter {Name='ppl_runner'} | Select-Object -Property ExitCode
if ($serviceExitCode -ne 0) {
    Write-Warning "ppl_runner service failed to run the executable with exit code $serviceExitCode"
    exit 1
}

Write-Host "'$fullPathToExecutable $CommandLineArgs' successfully ran as a PPL!"
