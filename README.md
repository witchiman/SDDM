## INTRODUCTION

[![Build Status](https://travis-ci.org/sddm/sddm.svg?branch=master)](https://travis-ci.org/sddm/sddm)
[![IRC Network](https://img.shields.io/badge/irc-freenode-blue.svg "IRC Freenode")](https://webchat.freenode.net/?channels=sddm)

Space OS SDDM is a custom version based on the open source software of SDDM developed by KDE team.

## INTRODUCTION

SDDM is a modern display manager for X11 and Wayland aiming to be fast, simple and beautiful.
It uses modern technologies like QtQuick, which in turn gives the designer the ability to
create smooth, animated user interfaces.

SDDM is extremely themeable. We put no restrictions on the user interface design,
it is completely up to the designer. We simply provide a few callbacks to the user interface
which can be used for authentication, suspend etc.

To further ease theme creation we provide some premade components like a textbox,
a combox etc.

There are a few sample themes distributed with SDDM.
They can be used as a starting point for new themes.

## INSTALLATION

Qt >= 5.3.0 is required to use SDDM, although Qt >= 5.4.0 is recommended.

SDDM runs the greeter as a system user named "sddm" whose home directory needs
to be set to `/var/lib/sddm`.

If pam and systemd are available, the greeter will go through logind
which will give it access to drm devices.

Distributions without pam and systemd will need to put the "sddm" user
into the "video" group, otherwise errors regarding GL and drm devices
might be experienced.

## LICENSE

Source code of SDDM is licensed under GNU GPL version 2 or later (at your choosing).
QML files are MIT licensed and images are CC BY 3.0.

## TROUBLESHOOTING

### NVIDIA Prime

Add this at the bottom of the Xsetup script:

```sh
if [ -e /sbin/prime-offload ]; then
    echo running NVIDIA Prime setup /sbin/prime-offload, you will need to manually run /sbin/prime-switch to shut down
    /sbin/prime-offload
fi
```

### No User Icon

SDDM reads user icon from either ~/.face.icon or FacesDir/username.face.icon

You need to make sure that SDDM user have permissions to read those files.
In case you don't want to allow other users to access your $HOME you can use
ACLs if your filesystem does support it.

```sh
setfacl -m u:sddm:x /home/username
setfacl -m u:sddm:r /home/username/.face.icon
```
