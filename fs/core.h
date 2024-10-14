#pragma once

#include <deque>
#include <expected>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace Filesystem {

enum class Error
{
  Generic = 1,
  FileFound,
  DirectoryFound,
  FileNotFound,
  DirectoryNotFound,
  InvalidPath,
  Unknown,
};

enum class NodeType
{
  DIR = 1,
  FILE,
};

class Node
{
public:
  Node() = default;

  virtual void printRecursive() const = 0;
  virtual void printRecursive(int level) const = 0;
  virtual bool isFile() const = 0;

  size_t getSize() const;

private:
  size_t size;
};

class File : Node
{
public:
  File(const std::string& name);
  bool isFile() const;

  void printRecursive() const;
  void printRecursive(int level) const;

  const std::string name;
};

class Directory : Node
{
public:
  Directory(const std::string& name);
  bool isFile() const;

  void printRecursive() const;
  void printRecursive(int level) const;

  std::optional<Error> addFile(File& file);
  std::optional<Error> addDir(Directory& dir);

  // std::expected<File, Error> getFile(const std::string& name);
  // std::expected<Directory, Error> getDirectory(const std::string& name);

  const std::string name;

private:
  std::map<std::string, File> files;
  std::map<std::string, Directory> dirs;
};

class Filesystem
{
public:
  Filesystem();
  ~Filesystem() = default;

  void printRecursive() const;

  void scan(const std::string& base);

  std::optional<Node*> find(const std::string& path);

  std::optional<Error> mkdir(const std::string& path);
  std::optional<Error> rmdir(const std::string& path);

private:
  Directory root;

  std::unique_ptr<Directory> cwd;
};

}