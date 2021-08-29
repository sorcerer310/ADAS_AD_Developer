#include "datamodel/aiccdisplaydatamodel.hpp"

AICCDisplayDataModel::AICCDisplayDataModel():_label(new QLabel("Resulting Text"))
{
    _label->setMargin(3);
}

unsigned int AICCDisplayDataModel::nPorts(PortType portType) const
{
    unsigned int result = 1;
    switch(portType)
    {
    case PortType::In:
        result = 1;
        break;
    case PortType::Out:
        result = 0;
    default:
        break;
    }
    return result;
}

NodeDataType AICCDisplayDataModel:: dataType(PortType,PortIndex) const
{
    return TextData().type();
}

std::shared_ptr<NodeData> AICCDisplayDataModel::outData(PortIndex)
{
    std::shared_ptr<NodeData> ptr;
    return ptr;
}
