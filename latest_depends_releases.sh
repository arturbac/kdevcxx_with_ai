#!/bin/bash

# Check if jq is installed
if ! command -v jq &> /dev/null; then
    echo "'jq' could not be found. Please install it to use this script."
    echo "You can install 'jq' on Gentoo using: sudo emerge app-misc/jq"
    exit 1
fi

declare -a repos=("fmtlib/fmt" 
  "gabime/spdlog"
  "stephenberry/glaze"
  "boost-ext/ut"
#    "arturbac/simple_enum" "arturbac/small_vectors" "arturbac/stralgo" "arturbac/fixed_math"
   )

# Calculate max lengths for formatting
max_repo_len=$(printf "%s\n" "${repos[@]}" | awk '{ if (length > max) max = length } END { print max }')
max_tag_len=11 # "Latest Tag" length
max_release_len=15 # "Latest Release" length

# Print top bar
printf "+-%-${max_repo_len}s-+-%-${max_tag_len}s-+-%-${max_release_len}s-+\n" | tr " " "-"

# Print headers
printf "| %-${max_repo_len}s | %-${max_tag_len}s | %-${max_release_len}s |\n" "Repository" "Latest Tag" "Latest Release"

# Print separator
printf "+-%-${max_repo_len}s-+-%-${max_tag_len}s-+-%-${max_release_len}s-+\n" | tr " " "-"

# Fetch and print each repository's latest tag and release
for repo in "${repos[@]}"; do
  response=$(curl -s "https://api.github.com/repos/$repo/tags")
  if echo "$response" | grep -q "API rate limit exceeded"; then
    latest_tag="Rate limit exceeded"
  else
    latest_tag=$(echo "$response" | jq -r '.[0].name // "None"')
  fi

  response=$(curl -s "https://api.github.com/repos/$repo/releases/latest")
  if echo "$response" | grep -q "API rate limit exceeded"; then
    latest_release="Rate limit exceeded"
  else
    latest_release=$(echo "$response" | jq -r '.tag_name // "None"')
  fi

  printf "| %-${max_repo_len}s | %-${max_tag_len}s | %-${max_release_len}s |\n" "$repo" "$latest_tag" "$latest_release"
done

# Print bottom bar
printf "+-%-${max_repo_len}s-+-%-${max_tag_len}s-+-%-${max_release_len}s-+\n" | tr " " "-"
