# Praktikum Hardwarenahe Systementwicklung

Compile:

```bash
make
```

Flashing:

```bash
avrdude -c usbasp -p AT89S52 -B10 -D -e -Uflash:w:build/main.ihx:i
```