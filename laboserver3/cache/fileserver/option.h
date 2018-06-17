#ifndef OPTION_H
#define OPTION_H
template <class T>
class Option
{
    bool _hasValue;

    QList<T> _value;

public:
    bool hasValue() {
        return _hasValue;
    }

    T value() {
        return _value[0];
    }

    static Option<T> some(const T& value) {
        return Option(value);
    }
    static Option<T> none() {
        return Option();
    }

private:
    Option(): _hasValue(false) {}
    Option(const T& value): _hasValue(true), _value(QList<T>() << value){}
};
#endif // OPTION_H
