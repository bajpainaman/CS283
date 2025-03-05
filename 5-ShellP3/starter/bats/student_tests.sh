#!/usr/bin/env bats

@test "Single command: ls runs without error" {
  run ../dsh <<EOF
ls
exit
EOF
  [ "$status" -eq 0 ]
}

@test "Pipeline: ls | grep \".c\" returns output" {
  run ../dsh <<EOF
ls | grep ".c"
exit
EOF
  [ "$status" -eq 0 ]
  [ -n "$output" ]
}

@test "Built-in command: dragon outputs expected string" {
  run ../dsh <<EOF
dragon
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "Here be dragons" ]]
}

@test "Built-in command: cd changes directory to HOME" {
  run ../dsh <<EOF
cd
pwd
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "$HOME" ]]
}

@test "Redirection: output redirection (>) creates file with correct content" {
  run bash -c "rm -f out.txt"
  run ../dsh <<EOF
echo "hello, class" > out.txt
cat out.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello, class" ]]
}

@test "Redirection: append redirection (>>) appends output correctly" {
  run bash -c "echo 'hello, class' > out.txt"
  run ../dsh <<EOF
echo "this is line 2" >> out.txt
cat out.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello, class" ]]
  [[ "$output" =~ "this is line 2" ]]
}

@test "Exit command terminates shell with appropriate message" {
  run ../dsh <<EOF
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "exiting" ]]
}
