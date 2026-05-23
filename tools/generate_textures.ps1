# Tao texture PNG cho du an mo phong cau truc
$outDir = Join-Path (Split-Path $PSScriptRoot -Parent) "assets\textures"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

Add-Type -AssemblyName System.Drawing

function Save-Texture {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [string]$Name
    )
    $path = Join-Path $outDir $Name
    $Bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $Bitmap.Dispose()
    Write-Host "Created $path"
}

function New-GroundTexture {
    param([int]$Size = 512)
    $bmp = New-Object System.Drawing.Bitmap $Size, $Size
    $rand = New-Object System.Random 42

    for ($y = 0; $y -lt $Size; $y++) {
        for ($x = 0; $x -lt $Size; $x++) {
            $n = $rand.Next(0, 35)
            $base = 95 + $n
            $r = [Math]::Min(255, $base + 8)
            $g = [Math]::Min(255, $base + 2)
            $b = [Math]::Max(0, $base - 12)

            # Duong ke luoi nhe
            if (($x % 64 -lt 2) -or ($y % 64 -lt 2)) {
                $r = [Math]::Min(255, $r + 25)
                $g = [Math]::Min(255, $g + 25)
                $b = [Math]::Min(255, $b + 25)
            }

            # Vung dam hon ngau nhien
            if ($rand.Next(0, 80) -eq 0) {
                $r = [Math]::Max(0, $r - 20)
                $g = [Math]::Max(0, $g - 20)
                $b = [Math]::Max(0, $b - 20)
            }

            $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb($r, $g, $b))
        }
    }
    return $bmp
}

function New-MetalTexture {
    param([int]$Size = 512)
    $bmp = New-Object System.Drawing.Bitmap $Size, $Size

    for ($y = 0; $y -lt $Size; $y++) {
        for ($x = 0; $x -lt $Size; $x++) {
            $nx = $x / [double]$Size
            $ny = $y / [double]$Size
            $stripe = 0.5 + 0.5 * [Math]::Sin($nx * 40 + $ny * 8)
            $scratch = 0.5 + 0.5 * [Math]::Sin($ny * 120)
            $v = 0.35 + 0.25 * $stripe + 0.1 * $scratch
            $c = [int]([Math]::Min(255, $v * 255))
            $r = $c
            $g = [int]($c * 0.95)
            $b = [int]($c * 0.85)
            $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb($r, $g, $b))
        }
    }
    return $bmp
}

function New-ConcreteTexture {
    param([int]$Size = 512)
    $bmp = New-Object System.Drawing.Bitmap $Size, $Size

    for ($y = 0; $y -lt $Size; $y++) {
        for ($x = 0; $x -lt $Size; $x++) {
            $noise = ($x * 13 + $y * 7 + ($x -bxor $y) * 3) % 40
            $c = 140 + $noise
            $r = [Math]::Min(255, $c)
            $g = [Math]::Min(255, $c - 5)
            $b = [Math]::Max(0, $c - 10)
            $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb($r, $g, $b))
        }
    }
    return $bmp
}

function New-WarningTexture {
    param([int]$Size = 512)
    $bmp = New-Object System.Drawing.Bitmap $Size, $Size
    $yellow = [System.Drawing.Color]::FromArgb(255, 210, 40)
    $black = [System.Drawing.Color]::FromArgb(25, 25, 25)
    $stripe = 48

    for ($y = 0; $y -lt $Size; $y++) {
        for ($x = 0; $x -lt $Size; $x++) {
            $diag = [int](($x + $y) / $stripe) % 2
            $bmp.SetPixel($x, $y, $(if ($diag -eq 0) { $yellow } else { $black }))
        }
    }
    return $bmp
}

function New-CargoTexture {
    param([int]$Size = 256)
    $bmp = New-Object System.Drawing.Bitmap $Size, $Size
    $wood1 = [System.Drawing.Color]::FromArgb(160, 110, 70)
    $wood2 = [System.Drawing.Color]::FromArgb(130, 85, 50)

    for ($y = 0; $y -lt $Size; $y++) {
        for ($x = 0; $x -lt $Size; $x++) {
            $band = [int]($y / 16) % 2
            $c = if ($band -eq 0) { $wood1 } else { $wood2 }
            if ($x % 32 -lt 2) {
                $c = [System.Drawing.Color]::FromArgb(
                    [Math]::Max(0, $c.R - 30),
                    [Math]::Max(0, $c.G - 30),
                    [Math]::Max(0, $c.B - 30))
            }
            $bmp.SetPixel($x, $y, $c)
        }
    }
    return $bmp
}

Save-Texture (New-GroundTexture) "ground.png"
Save-Texture (New-MetalTexture) "metal.png"
Save-Texture (New-ConcreteTexture) "concrete.png"
Save-Texture (New-WarningTexture) "warning.png"
Save-Texture (New-CargoTexture) "cargo.png"

Write-Host "Done. 5 textures in $outDir"
