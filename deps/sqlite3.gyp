{ 'targets': [
    {
      'target_name': 'sqlite3',
      'type': 'static_library',
      'sources': [
        "sqlite3/sqlite3.c"
      ],
      'defines': [
        'HAVE_USLEEP=1',
        # This is important - it causes SQLite to use memory for temp files. Since
        # Android has no globally writable temp directory, if this is not defined the
        # application throws an exception when it tries to create a temp file.
        'SQLITE_TEMP_STORE=3',
      ],
      'cflags': [
        '-DHAVE_USLEEP=1',
        '-DSQLITE_TEMP_STORE=3',
        '-Wno-unused-const-variable',
        '-Wno-unused-parameter',
      ],
      'cflags!': [
        '-Werror',
        '-Wextra',
      ],
      'xcode_settings': {
        'OTHER_CFLAGS' : ['<@(_cflags)'],
        'OTHER_CFLAGS!' : ['-Wextra', '-Werror'],
      },
      'all_dependent_settings': {
        'include_dirs': [
          '.',
          'sqlite3',
        ]
      },
      'include_dirs': [
        'sqlite3',
      ],
    },
  ]
}
