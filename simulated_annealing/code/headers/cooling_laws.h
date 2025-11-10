using namespace std;

// ------------------------------ Законы понижения температуры ------------------------------
// Закон Больцмана: T = T0 / ln(1 + i)
class BoltzmannCooling : public CoolingLaw {
    private:
        double T0_;
    public:
        BoltzmannCooling(double T0) : T0_(T0) {}
        
        double nextTemperature(double currentT, int iter) override {
            if (iter == 0) return T0_;
            return T0_ / std::log(1 + iter);
        }
    };
    
    // Закон Коши: T = T0 / (1 + i)
    class CauchyCooling : public CoolingLaw {
    private:
        double T0_;
    public:
        CauchyCooling(double T0) : T0_(T0) {}
        
        double nextTemperature(double currentT, int iter) override {
            return T0_ / (1 + iter);
        }
    };
    
    // Смешанный закон: T = T0 * ln(1 + i) / (1 + i)
    class MixedCooling : public CoolingLaw {
    private:
        double T0_;
    public:
        MixedCooling(double T0) : T0_(T0) {}
        
        double nextTemperature(double currentT, int iter) override {
            if (iter == 0) return T0_;
            return T0_ * std::log(1 + iter) / (1 + iter);
        }
    };