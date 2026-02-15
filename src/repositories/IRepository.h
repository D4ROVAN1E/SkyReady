#ifndef IREPOSITORY_H
#define IREPOSITORY_H

#include <vector>
#include <QUuid>

// Шаблонный интерфейс для CRUD операций
template <typename T>
class IRepository {
public:
    virtual ~IRepository() {}
    virtual std::vector<T> getAll() = 0;
    virtual T getById(QUuid id) = 0;
};

#endif // IREPOSITORY_H
