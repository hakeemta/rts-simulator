#ifndef RESOURCE_HPP
#define RESOURCE_HPP

class Resource {
public:
  Resource(){};
  Resource(int id) : _id(id) {}
  int id() const { return _id; };

protected:
  int _id;
};

#endif