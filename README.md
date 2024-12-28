<picture>
  <source media="(prefers-color-scheme: dark)" srcset=".github/readme_banner_white.png">
  <source media="(prefers-color-scheme: light)" srcset=".github/readme_banner_black.png">
  <img alt="LimeOS Banner">
</picture>

###

This window manager for X11 seamlessly blends the best of both tiled and 
floating window management. Designed with minimalism and elegance in mind,
it aims to provide a clean and uncluttered interface while still delivering all
the essential functionalities that users expect from a modern window manager.

> **NOTE:** This project is still in development and is not ready for use.

## Building & Running

To build this project locally, you will need the following dependencies:

```bash
# The following command is intended for Debian based systems.
sudo apt install \
    gcc \
    make \
    libx11-dev \
    libxi-dev \
    libcairo2-dev
```

Once the dependencies are installed, you can build the project by running:

```bash
make
```

This will compile the source code and generate an executable in the `./bin`
directory.

You can now either run the executable directly in order to use it as your
primary window manager, or you can use a nested X server like `Xephyr` in order
to run the window manager within your currently active window manager.

> **NOTE:** Testing this window manager within a Wayland compositor may cause
conflicts. We recommend using an X11-based window manager as your parent
environment to prevent unexpected behavior.

```bash
Xephyr -br -ac -noreset -screen 800x600 :1
DISPLAY=:1 ./bin/lime-os-wm
```

Then if you'd like, you could start an application on the new display as follows:

```bash
# xterm is being used as an example here, replace it with whatever you'd like.
DISPLAY=:1 xterm &
```

## License

This project is licensed under the GPL-3.0 License. This license reflects our 
commitment to ensuring that this software remains free and open-source. 
We believe in the values of freedom, transparency, and collaboration that the 
GPL-3.0 promotes, allowing users to freely use, modify, and distribute the 
software, ensuring that it remains a community-driven project.

For more details, see the `LICENSE.md` file.
