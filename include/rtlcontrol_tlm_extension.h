#ifndef RTLCONTROLTLMEXTENSION_H
#define RTLCONTROLTLMEXTENSION_H

//We implement TLM2.0 custom extension that carries "req_delay" and "resp_delay" property. Using these we can let transactors control backpressure when communicating with an RTL model
//Although using TLM2.0 extension looks extensive for just an addition of two extra parameters to a TLM generic payload but this is recommended and compliant way
class rtlcontrol_tlm_extension : public tlm::tlm_extension<rtlcontrol_tlm_extension> {
public:
    sc_time req_delay = SC_ZERO_TIME;
    sc_time resp_delay = SC_ZERO_TIME;
    //// start of overriden pure virtual functions from tlm_extension (this is common way of overriding them when creating custom extension) ////
    tlm_extension_base* clone() const override {
        auto* ext = new rtlcontrol_tlm_extension();
        ext->req_delay = this->req_delay;
        ext->resp_delay = this->resp_delay;
        return ext;
    }

    void copy_from(tlm_extension_base const& other) override {
        auto const& ext = static_cast<const rtlcontrol_tlm_extension&>(other);
        req_delay = ext.req_delay;
        resp_delay = ext.resp_delay;
    }
    //// end of overriden pure virtual functions from tlm_extension ////
};

#endif
