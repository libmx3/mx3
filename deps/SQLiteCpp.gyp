{ 'targets': [
    {
      'target_name': 'SQLiteCpp',
      'type': 'static_library',
      'sources': [
        "SQLiteCpp/src/Database.cpp",
        "SQLiteCpp/src/Statement.cpp",
        "SQLiteCpp/src/Transaction.cpp",
      ],
      'cflags': [
        '-DHAVE_USLEEP=1',
        '-Wno-unused-const-variable',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '.',
          './sqlite3',
        ]
      },
      'include_dirs': [
        'SQLiteCpp/include',
      ],
    },
  ]
}
