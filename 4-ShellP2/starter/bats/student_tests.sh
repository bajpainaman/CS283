#!/usr/bin/env bats

# File: student_tests.sh
# 
# Unit tests for dsh shell implementation

setup() {
    # Ensure dsh is executable
    chmod +x ./dsh
}

@test "Check 'ls' runs without errors" {
    run ./dsh <<EOF
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Check 'cd' with no arguments does nothing" {
    run ./dsh <<EOF
cd
EOF
    [ "$status" -eq 0 ]
}

@test "Check 'cd' to valid directory" {
    run ./dsh <<EOF
cd /tmp
EOF
    [ "$status" -eq 0 ]
}

@test "Check 'cd' to invalid directory" {
    run ./dsh <<EOF
cd /nonexistent_directory
EOF
    [ "$status" -ne 0 ]
}

@test "Check 'rc' prints last exit code" {
    run ./dsh <<EOF
ls
rc
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ ^0$ ]]  # Expecting last exit code of 0
}

@test "Check 'dragon' command prints ASCII art" {
    run ./dsh <<EOF
dragon
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Drexel Dragon" ]]  # Change this based on expected output
}

@test "Check 'exit' command terminates shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Check invalid command gives error" {
    run ./dsh <<EOF
invalid_command
EOF
    [ "$status" -ne 0 ]
    [[ "$output" =~ "Command not found" ]]
}

@test "Check external command execution (echo test)" {
    run ./dsh <<EOF
echo Hello, World!
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "Hello, World!" ]]
}

@test "Check handling of empty input" {
    run ./dsh <<EOF

EOF
    [ "$status" -eq 0 ]
}

