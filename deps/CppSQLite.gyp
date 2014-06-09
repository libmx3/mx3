{ 'targets': [
    {
        'target_name': 'CppSQLite',
        'type': 'static_library',
        'sources': [
            "CppSQLite/CppSQLite3.cpp"
        ],
        # disable sign compare warnings, since CppSQLite3 has a few of them
        'cflags': [
            '-Wno-maybe-uninitialized',
        ],
        'all_dependent_settings': {
            'include_dirs': [
                '.',
            ]
        },
        'include_dirs': [
            'sqlite3/',
            'CppSQLite',
        ],
    },
]}
