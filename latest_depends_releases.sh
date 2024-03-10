#!/bin/bash

# Check if jq is installed
if ! command -v jq &> /dev/null
then
    echo "'jq' could not be found. Please install it to use this script."
    echo "You can install 'jq' on Gentoo using: sudo emerge app-misc/jq"
    exit 1
fi

# Repositories to check
# Repositories to check
declare -a repos=(
  "fmtlib/fmt"
  "gabime/spdlog"
  "stephenberry/glaze"
  "arturbac/simple_enum"
  "arturbac/small_vectors"
  "arturbac/stralgo"
  "arturbac/fixed_math"
)


# Headers
header1="Repository"
header2="Latest Tag"
header3="Latest Release"

# Find longest repository name for formatting
max_len=${#header1}
for repo in "${repos[@]}"; do
  [ ${#repo} -gt $max_len ] && max_len=${#repo}
done

# Print header with dynamic spacing
printf "| %-${max_len}s | %-${#header2}s | %-${#header3}s |\n" "$header1" "$header2" "$header3"
printf "|%s|%s|%s|\n" $(printf '=%.0s' $(seq 1 $((max_len + 2)))) $(printf '=%.0s' $(seq 1 $((${#header2} + 2)))) $(printf '=%.0s' $(seq 1 $((${#header3} + 2))))

# Fetch and print each repository's latest tag and release
for repo in "${repos[@]}"; do
  response=$(curl -s "https://api.github.com/repos/$repo/tags")
  if echo "$response" | grep -q "API rate limit exceeded"; then
    echo "API rate limit exceeded. Please try again later."
    exit 1
  fi
  latest_tag=$(echo "$response" | jq -r '.[0].name')

  response=$(curl -s "https://api.github.com/repos/$repo/releases/latest")
  if echo "$response" | grep -q "API rate limit exceeded"; then
    echo "API rate limit exceeded. Please try again later."
    exit 1
  fi
  latest_release=$(echo "$response" | jq -r '.tag_name')
  
  printf "| %-${max_len}s | %-${#header2}s | %-${#header3}s |\n" "$repo" "$latest_tag" "$latest_release"
done

