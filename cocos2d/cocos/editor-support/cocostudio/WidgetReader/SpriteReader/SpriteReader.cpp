/****************************************************************************
 Copyright (c) 2014 cocos2d-x.org
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "editor-support/cocostudio/WidgetReader/SpriteReader/SpriteReader.h"

#include "2d/CCSprite.h"
#include "2d/CCSpriteFrameCache.h"
#include "platform/CCFileUtils.h"

#include "editor-support/cocostudio/CSParseBinary_generated.h"
#include "editor-support/cocostudio/FlatBuffersSerialize.h"
#include "editor-support/cocostudio/WidgetReader/NodeReader/NodeReader.h"


#include "tinyxml2.h"
#include "flatbuffers/flatbuffers.h"

USING_NS_CC;
using namespace flatbuffers;

namespace cocostudio
{
    IMPLEMENT_CLASS_NODE_READER_INFO(SpriteReader)
    
    SpriteReader::SpriteReader()
    {
        
    }
    
    SpriteReader::~SpriteReader()
    {
        
    }
    
    static SpriteReader* _instanceSpriteReader = nullptr;
    
    SpriteReader* SpriteReader::getInstance()
    {
        if (!_instanceSpriteReader)
        {
            _instanceSpriteReader = new (std::nothrow) SpriteReader();
        }
        
        return _instanceSpriteReader;
    }
    
    void SpriteReader::purge()
    {
        CC_SAFE_DELETE(_instanceSpriteReader);
    }
    
    void SpriteReader::destroyInstance()
    {
        CC_SAFE_DELETE(_instanceSpriteReader);
    }
    
    Offset<Table> SpriteReader::createOptionsWithFlatBuffers(const tinyxml2::XMLElement *objectData,
                                                             flatbuffers::FlatBufferBuilder *builder)
    {
        auto temp = NodeReader::getInstance()->createOptionsWithFlatBuffers(objectData, builder);
        auto nodeOptions = *(Offset<WidgetOptions>*)(&temp);
        
        std::string path = "";
        std::string plistFile = "";
        int resourceType = 0;
        
        cocos2d::BlendFunc blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
        
        // FileData
        const tinyxml2::XMLElement* child = objectData->FirstChildElement();
        while (child)
        {
            std::string name = child->Name();
            
            if (name == "FileData")
            {
                std::string texture = "";
                std::string texturePng = "";
                
                const tinyxml2::XMLAttribute* attribute = child->FirstAttribute();
                
                while (attribute)
                {
                    name = attribute->Name();
                    std::string value = attribute->Value();
                    
                    if (name == "Path")
                    {
                        path = value;
                    }
                    else if (name == "Type")
                    {
                        resourceType = getResourceType(value);
                    }
                    else if (name == "Plist")
                    {
                        plistFile = value;
                        texture = value;
                    }
                    
                    attribute = attribute->Next();
                }
                
                if (resourceType == 1)
                {
                    FlatBuffersSerialize* fbs = FlatBuffersSerialize::getInstance();
                    fbs->_textures.push_back(builder->CreateString(texture));                    
                }
            }
            else if (name == "BlendFunc")
            {
                const tinyxml2::XMLAttribute* attribute = child->FirstAttribute();
                
                while (attribute)
                {
                    name = attribute->Name();
                    std::string value = attribute->Value();
                    
                    if (name == "Src")
                    {
                        blendFunc.src = atoi(value.c_str());
                    }
                    else if (name == "Dst")
                    {
                        blendFunc.dst = atoi(value.c_str());
                    }
                    
                    attribute = attribute->Next();
                }
            }
            
            child = child->NextSiblingElement();
        }
        
        flatbuffers::BlendFunc f_blendFunc(blendFunc.src, blendFunc.dst);

        auto options = CreateSpriteOptions(*builder,
                                           nodeOptions,
                                           CreateResourceData(*builder,
                                                              builder->CreateString(path),
                                                              builder->CreateString(plistFile),
                                                              resourceType),
                                           &f_blendFunc);
        
        return *(Offset<Table>*)(&options);
    }
    
    void SpriteReader::setPropsWithFlatBuffers(cocos2d::Node *node,
                                                   const flatbuffers::Table* spriteOptions)
    {
        Sprite *sprite = static_cast<Sprite*>(node);
        auto options = (SpriteOptions*)spriteOptions;
        
        auto nodeReader = NodeReader::getInstance();
        nodeReader->setPropsWithFlatBuffers(node, (Table*)(options->nodeOptions()));
        
        auto fileNameData = options->fileNameData();
        
        int resourceType = fileNameData->resourceType();
        std::string path = fileNameData->path()->c_str();
        
        std::string errorFilePath = "";
        
        switch (resourceType)
        {
            case 0:
            {
                if (FileUtils::getInstance()->isFileExist(path))
                {
                    sprite->setTexture(path);
                }
                else
                {
                    errorFilePath = path;
                }
                break;
            }
                
            case 1:
            {
                std::string plist = fileNameData->plistFile()->c_str();
                SpriteFrame* spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(path);
                if (spriteFrame)
                {
                    sprite->setSpriteFrame(spriteFrame);
                }
                else
                {
                    if (FileUtils::getInstance()->isFileExist(plist))
                    {
                        ValueMap value = FileUtils::getInstance()->getValueMapFromFile(plist);
                        ValueMap metadata = value["metadata"].asValueMap();
                        std::string textureFileName = metadata["textureFileName"].asString();
                        if (!FileUtils::getInstance()->isFileExist(textureFileName))
                        {
                            errorFilePath = textureFileName;
                        }
                    }
                    else
                    {
                        errorFilePath = plist;
                    }
                }
                break;
            }
                
            default:
                break;
        }
        
        auto f_blendFunc = options->blendFunc();
        if (f_blendFunc)
        {
            cocos2d::BlendFunc blendFunc = cocos2d::BlendFunc::ALPHA_PREMULTIPLIED;
            blendFunc.src = f_blendFunc->src();
            blendFunc.dst = f_blendFunc->dst();
            sprite->setBlendFunc(blendFunc);
        }
        
        auto nodeOptions = options->nodeOptions();
        
        GLubyte alpha       = (GLubyte)nodeOptions->color()->a();
        GLubyte red         = (GLubyte)nodeOptions->color()->r();
        GLubyte green       = (GLubyte)nodeOptions->color()->g();
        GLubyte blue        = (GLubyte)nodeOptions->color()->b();
        
        if (alpha != 255)
        {
            sprite->setOpacity(alpha);
        }
        if (red != 255 || green != 255 || blue != 255)
        {
            sprite->setColor(Color3B(red, green, blue));
        }
        
        bool flipX   = nodeOptions->flipX() != 0;
        bool flipY   = nodeOptions->flipY() != 0;
        
        if(flipX != false)
            sprite->setFlippedX(flipX);
        if(flipY != false)
            sprite->setFlippedY(flipY);
    }
    
    Node* SpriteReader::createNodeWithFlatBuffers(const flatbuffers::Table *spriteOptions)
    {
        Sprite* sprite = Sprite::create();
        
        setPropsWithFlatBuffers(sprite, (Table*)spriteOptions);
        
        return sprite;
    }
    
    int SpriteReader::getResourceType(const std::string& key)
    {
        if(key == "Normal" || key == "Default")
        {
            return 	0;
        }
        
        FlatBuffersSerialize* fbs = FlatBuffersSerialize::getInstance();
        if(fbs->_isSimulator)
        {
            if(key == "MarkedSubImage")
            {
                return 0;
            }
        }
        return 1;
    }
}