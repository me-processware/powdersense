# PowerShell script to generate checksums for PowderSense firmware files
# Usage: Run this script in the releases/v1.0/ directory

Write-Host "=== PowderSense v1.0 Checksum Generator ===" -ForegroundColor Cyan
Write-Host ""

# Check if files exist
$files = @("bootloader.bin", "partitions.bin", "firmware.bin", "spiffs.bin")
$missingFiles = @()

foreach ($file in $files) {
    if (-not (Test-Path $file)) {
        $missingFiles += $file
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Host "ERROR: Missing files:" -ForegroundColor Red
    foreach ($file in $missingFiles) {
        Write-Host "  - $file" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "Please copy binaries from .pio/build/esp32c6_touch/ first!" -ForegroundColor Yellow
    exit 1
}

# Generate checksums
Write-Host "Generating checksums..." -ForegroundColor Green
Write-Host ""

$output = @()
$output += "# PowderSense v1.0 - Firmware Checksums"
$output += ""
$output += "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$output += ""

foreach ($file in $files) {
    Write-Host "Processing $file..." -ForegroundColor Yellow
    
    # Get file size
    $size = (Get-Item $file).Length
    $sizeKB = [math]::Round($size / 1KB, 2)
    $sizeMB = [math]::Round($size / 1MB, 2)
    
    # Calculate MD5
    $md5 = (Get-FileHash -Path $file -Algorithm MD5).Hash
    
    # Calculate SHA256
    $sha256 = (Get-FileHash -Path $file -Algorithm SHA256).Hash
    
    # Add to output
    $output += "## $file"
    $output += ""
    $output += "- **Size**: $size bytes ($sizeKB KB / $sizeMB MB)"
    $output += "- **MD5**: ``$md5``"
    $output += "- **SHA256**: ``$sha256``"
    $output += ""
    
    Write-Host "  Size: $sizeKB KB" -ForegroundColor Gray
    Write-Host "  MD5: $md5" -ForegroundColor Gray
    Write-Host "  SHA256: $sha256" -ForegroundColor Gray
    Write-Host ""
}

# Save to file
$outputFile = "CHECKSUMS.md"
$output | Out-File -FilePath $outputFile -Encoding UTF8

Write-Host "=== Checksums saved to $outputFile ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Review CHECKSUMS.md" -ForegroundColor White
Write-Host "2. Copy checksums to README.md" -ForegroundColor White
Write-Host "3. Delete PLACE_BINARIES_HERE.txt" -ForegroundColor White
Write-Host "4. Commit: git add . && git commit -m 'release: add v1.0 binaries'" -ForegroundColor White
Write-Host "5. Tag: git tag v1.0" -ForegroundColor White
Write-Host "6. Push: git push origin master --tags" -ForegroundColor White
Write-Host ""
