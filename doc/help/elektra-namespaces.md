elektra-namespaces(7) -- namespaces
===================================

## INTRODUCTION

Every key in Elektra has a unique name. Sometimes, multiple keys denote
the same configuration item from different sources, e.g.:

- by a commandline argument
- by a configuration file found below the home directory
- by a configuration file found below /etc

To allow such keys to exist in parallel, Elektra uses namespaces.

A namespace has following properties:

- in-memory Keys can start with this name
- these keys stem from a specific configuration source
- ksLookup() uses namespaces in a specific default order unless
    specified otherwise


Following parts of Elektra are affected by namespaces:

- the key name validation in keySetName()
- keyGetNamespace() which enumerates all namespaces
- _Backend and split.c for correct distribution to plugins (note that
    not all namespaces actually are distributed to configuration files)
- mount.c for cascading and root backends
- and of course many unit tests

In the rest of this document all currently available namespaces in the default order
are described.



## spec

Unlike the other namespaces, the specification namespace does not
contain values of the keys, but instead meta data as described in
[METADATA.ini](/doc/METADATA.ini).

When a key is looked up, keys from the spec-namespace are the first to
be searched. When a spec-key is found, the rest of the lookup will be
done as specified, probably in a different order than the namespaces
enlisted here.

Usually, the spec-keys do not directly contribute to the value, with one
notable exception: the default value (meta data `default`, see in
cascading below) might be used if every other way as specified in the
spec-key failed.

Spec-keys typically include a explanation and description for the key
itself (but not comments which are specific for individual keys).

The spec configuration files are below CMAKE_INSTALL_PREFIX/KDB_DB_SPEC.

spec is not part of cascading mounts, because the specifications often
are written in different syntax than the configuration files.


## proc

Derived from the process (e.g. by parsing /proc/self or by arguments passed
from the main method):

- program name
- arguments
- environment

Keys in the namespace proc can not be stored by their nature. They might
be different for every invocation of an application.


## dir

Keys from the namespace `dir` are derived from a directory special to
the user starting/using the application, e.g.:

- the current working directory for project specific settings, e.g. .git
- the directory a server wants to present to the user, e.g. .htaccess

Note that Elektra only supports a single special directory per KDB
instance. Start a new KDB instance if you need different special
directories for different parts of your application.
How to change the directory may be different dependent on the resolver,
e.g. by using chdir or by setting the environment variable PWD


## user

On multi-user operating systems obviously every user wants her/his own
configuration. The user configuration is located in the users home
directory typically below the folder KDB_DB_USER.
Other paths below the home directory are possible too (absolute path
for resolver).

Note that Elektra only supports a user directory per KDB
instance. Start a new KDB instance if you need different user
configuration for different parts of your application.
How to change the user may be different dependent on the resolver,
e.g. by seteuid() or by environment variables like HOME, USER


## system

The system configuration is the same for every chroot.

The configuration is typically located below KDB_DB_SYSTEM.
Other absolute paths, e.g. below /opt or /usr/local/etc are possible
too.


## cascading

Keys that are not in a namespace (i.e. start with an `/`) are called cascading
keys. Cascading keys do not stem from a configuration source, but are
used by applications to lookup a key in different namespaces.  So,
multiple keys can contribute to each cascading key name.

Cascading is the same as a name resolution and provides a
namespace unification as described in
[Versatility and Unix semantics in namespace unification](http://dl.acm.org/citation.cfm?id=1138045).

Keys without a namespace can not be stored by their nature. So they
are transient: after a restart they are forgotten.

Keys of that namespace are only used by ksLookup when no other suitable
key was found. So they have the lowest possible priority, even fallback
keys are preferred.

[Read more about cascading.](/doc/help/elektra-cascading.md)
