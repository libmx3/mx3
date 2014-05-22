{ 'targets': [
    {
        'target_name': 'CppSQLite',
        'type': 'static_library',
        'sources': [
            "CppSQLite/CppSQLite3.cpp"
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
