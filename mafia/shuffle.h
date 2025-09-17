template<typename T>
void simple_shuffle(T& container) {
    std::mt19937 g(rand());
    std::shuffle(container.begin(), container.end(), g);
}