#include "core.h"

#include <expected>
#include <optional>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <fmt/core.h>

namespace Filesystem {

size_t
Node::getSize() const
{
  return size;
}

///
/// File
///

File::File(const std::string& name)
  : name(name)
{
}

bool
File::isFile() const
{
  return true;
}

void
File::printRecursive() const
{
  printRecursive(0);
}

void
File::printRecursive(int level) const
{
  const std::string leader = std::string(level, ' ');
  fmt::print("{}[F]: {}\n", leader, name);
}

///
/// Directory
///

Directory::Directory(const std::string& name)
  : name(name)
{
}

bool
Directory::isFile() const
{
  return false;
}

void
Directory::printRecursive() const
{
  printRecursive(0);
}

void
Directory::printRecursive(int level) const
{
  const std::string leader = std::string(level, ' ');
  fmt::print("{}[D]: {}\n", leader, name);

  for (auto const& [_, obj] : files) {
    obj.printRecursive(level + 1);
  }
  for (auto const& [name, obj] : dirs) {
    obj.printRecursive(level + 1);
  }
}

std::optional<Error>
Directory::addFile(File& file)
{
  if (files.find(file.name) != files.end()) {
    return Error::FileFound;
  }

  files.insert({ file.name, file });
  return {};
}

std::optional<Error>
Directory::addDir(Directory& dir)
{
  if (dirs.find(dir.name) != dirs.end()) {
    return Error::DirectoryFound;
  }

  dirs.insert({ dir.name, dir });
  return {};
}

///
/// Filesystem
///

Filesystem::Filesystem()
  : root("/")
{
  cwd = std::make_unique<Directory>(root);

  File a = File("a");
  File b = File("b");
  File c = File("c");

  root.addFile(a);
  root.addFile(b);
  root.addFile(c);

  Directory sd1 = Directory("1");
  Directory sd2 = Directory("2");
  Directory sd3 = Directory("3");

  sd1.addFile(a);
  sd1.addFile(b);
  sd1.addFile(c);

  sd2.addFile(a);
  sd2.addFile(b);
  sd2.addFile(c);

  sd3.addFile(a);
  sd3.addFile(b);
  sd3.addFile(c);

  root.addDir(sd1);
  root.addDir(sd2);
  root.addDir(sd3);

  /*
  bool value = find("/").has_value();
  bool value2 = exists("/abcd/efgh/ijklmo/").has_value();
  fmt::print("{}\n", value);
  */
}

void
Filesystem::printRecursive() const
{
  root.printRecursive();
}

void
Filesystem::scan(const std::string& base)
{
  fmt::print("TODO {}: {}:{}\n", __LINE__, __FILE__, __FUNCTION__);
}

static bool
validPath(const std::string& path)
{
  if (!path.length()) {
    return false;
  }

  if (path.at(0) != '/') {
    return false;
  }

  return true;
}

std::optional<Node*>
Filesystem::find(const std::string& path)
{
  if (!validPath(path)) {
    return {};
  }

  if (path == "/") {
    return (Node*)&root;
  }

  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char>> tokens(path, sep);

  Directory* d = cwd.get();

  BOOST_FOREACH (const std::string& t, tokens) {
    fmt::print("{}\n", t);
  }

  return {};
}

std::optional<Error>
Filesystem::mkdir(const std::string& path)
{
  if (!validPath(path)) {
    return Error::InvalidPath;
  }

  return {};
}

std::optional<Error>
Filesystem::rmdir(const std::string& path)
{
  return {};
}

}