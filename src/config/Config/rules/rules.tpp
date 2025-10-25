template <typename U, typename V>
void replace(const U& value, V& target)
{
    target = value;
}

template <typename Value, typename Container>
void appendHead(const std::vector<Value>& value, Container& target)
{
    target.insert(target.begin(), value.begin(), value.end());
}
