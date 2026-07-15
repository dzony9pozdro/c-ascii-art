

## build
```bash
make            # compiles main.c -> main
make clean      # removes the binary
```

## run
requires imageMagick - run.sh prompts to install it

```bash
chmod +x run.sh
./run.sh yourimage.png # (any file format accepted by imageMagick)
```
don't run main directly - it expects 16b/channel P6 files in the format produced by run.sh
