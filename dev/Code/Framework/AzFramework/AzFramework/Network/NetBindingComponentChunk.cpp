/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <AzFramework/Network/NetBindingComponentChunk.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <AzFramework/Network/NetBindingSystemBus.h>
#include <AzFramework/Network/NetBindingEventsBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <GridMate/Serialize/Buffer.h>
#include <GridMate/Serialize/UuidMarshal.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Slice/SliceComponent.h>
#include <AzCore/Serialization/ObjectStream.h>
#include <AzCore/IO/ByteContainerStream.h>

namespace AzFramework
{
    NetBindingComponentChunk::SpawnInfo::SpawnInfo()
        : m_owningContextId(UnspecifiedNetBindingContextSequence)
    {
    }

    bool NetBindingComponentChunk::SpawnInfo::operator==(const SpawnInfo& rhs)
    {
        return m_owningContextId == rhs.m_owningContextId
            && m_runtimeEntityId == rhs.m_runtimeEntityId
            && m_staticEntityId == rhs.m_staticEntityId
            && m_serializedState == rhs.m_serializedState
            && m_sliceAssetId == rhs.m_sliceAssetId;
    }

    bool NetBindingComponentChunk::SpawnInfo::ContainsSerializedState() const
    {
        return !m_serializedState.empty();
    }

    void NetBindingComponentChunk::SpawnInfo::Marshaler::Marshal(GridMate::WriteBuffer& wb, const SpawnInfo& data)
    {
        wb.Write(data.m_owningContextId, GridMate::VlqU32Marshaler());
        wb.Write(data.m_runtimeEntityId);

        bool useSerializedState = data.ContainsSerializedState();
        wb.Write(useSerializedState);
        if (useSerializedState)
        {
            wb.Write(data.m_serializedState);
        }
        else
        {
            wb.Write(data.m_sliceAssetId);
            wb.Write(data.m_staticEntityId);
        }
    }
    
    void NetBindingComponentChunk::SpawnInfo::Marshaler::Unmarshal(SpawnInfo& data, GridMate::ReadBuffer& rb)
    {
        rb.Read(data.m_owningContextId, GridMate::VlqU32Marshaler());
        rb.Read(data.m_runtimeEntityId);

        bool hasSerializedState = false;
        rb.Read(hasSerializedState);
        if (hasSerializedState)
        {
            rb.Read(data.m_serializedState);
        }
        else
        {
            rb.Read(data.m_sliceAssetId);
            rb.Read(data.m_staticEntityId);
        }
    }

    NetBindingComponentChunk::NetBindingComponentChunk()
        : m_bindingComponent(nullptr)
        , m_spawnInfo("SpawnInfo")
        , m_bindMap("ComponentBindMap")
    {
        m_spawnInfo.SetMaxIdleTime(0.f);
        m_bindMap.SetMaxIdleTime(0.f);
    }

    void NetBindingComponentChunk::OnReplicaActivate(const GridMate::ReplicaContext& rc)
    {
        (void)rc;
        if (IsMaster())
        {
            // Get and store entity spawn data
            AZ_Assert(m_bindingComponent, "Entity binding is invalid!");

            m_spawnInfo.Modify([&](SpawnInfo& spawnInfo)
                {
                    spawnInfo.m_runtimeEntityId = static_cast<AZ::u64>(m_bindingComponent->GetEntity()->GetId());

                    bool isProceduralEntity = true;
                    AZ::SliceComponent::SliceInstanceAddress sliceInfo;

                    EntityContextId contextId = EntityContextId::CreateNull();
                    EBUS_EVENT_ID_RESULT(contextId, m_bindingComponent->GetEntityId(), EntityIdContextQueryBus, GetOwningContextId);
                    if (!contextId.IsNull())
                    {
                        EBUS_EVENT_RESULT(spawnInfo.m_owningContextId, NetBindingSystemBus, GetCurrentContextSequence);

                        EBUS_EVENT_ID_RESULT(sliceInfo, m_bindingComponent->GetEntityId(), EntityIdContextQueryBus, GetOwningSlice);
                        bool isDynamicSliceEntity = sliceInfo.first && sliceInfo.second;

                        isProceduralEntity = !m_bindingComponent->IsLevelSliceEntity() && !isDynamicSliceEntity;
                    }

                    if (isProceduralEntity)
                    {
                        // write cloning info
                        AZ::SerializeContext* sc = nullptr;
                        EBUS_EVENT_RESULT(sc, AZ::ComponentApplicationBus, GetSerializeContext);
                        AZ_Assert(sc, "Can't find SerializeContext!");
                        AZ::IO::ByteContainerStream<AZStd::vector<AZ::u8>> spawnDataStream(&spawnInfo.m_serializedState);
                        AZ::ObjectStream* objStream = AZ::ObjectStream::Create(&spawnDataStream, *sc, AZ::DataStream::ST_BINARY);
                        objStream->WriteClass(m_bindingComponent->GetEntity());
                        objStream->Finalize();
                    }
                    else
                    {
                        // write slice info
                        if (sliceInfo.first)
                        {
                            AZ::Data::AssetId sliceAssetId = sliceInfo.first->GetSliceAsset().GetId();
                            spawnInfo.m_sliceAssetId = AZStd::make_pair(sliceAssetId.m_guid, sliceAssetId.m_subId);
                        }

                        AZ::EntityId staticEntityId;
                        EBUS_EVENT_RESULT(staticEntityId, NetBindingSystemBus, GetStaticIdFromEntityId, m_bindingComponent->GetEntity()->GetId());
                        spawnInfo.m_staticEntityId = static_cast<AZ::u64>(staticEntityId);
                    }

                    return true;
                });
        }
        else
        {
            AZ::EntityId runtimeEntityId(m_spawnInfo.Get().m_runtimeEntityId);
            NetBindingContextSequence owningContextId = m_spawnInfo.Get().m_owningContextId;

            if (m_spawnInfo.Get().ContainsSerializedState())
            {
                // Spawn the entity from our data
                AZ::IO::MemoryStream spawnData(m_spawnInfo.Get().m_serializedState.data(), m_spawnInfo.Get().m_serializedState.size());
                EBUS_EVENT(NetBindingSystemBus, SpawnEntityFromStream, spawnData, runtimeEntityId, GetReplicaId(), owningContextId);
            }
            else
            {
                NetBindingSliceContext spawnContext;
                spawnContext.m_contextSequence = owningContextId;
                spawnContext.m_sliceAssetId = AZ::Data::AssetId(m_spawnInfo.Get().m_sliceAssetId.first, m_spawnInfo.Get().m_sliceAssetId.second);
                spawnContext.m_runtimeEntityId = runtimeEntityId;
                spawnContext.m_staticEntityId = AZ::EntityId(m_spawnInfo.Get().m_staticEntityId);
                EBUS_EVENT(NetBindingSystemBus, SpawnEntityFromSlice, GetReplicaId(), spawnContext);
            }
        }
    }

    void NetBindingComponentChunk::OnReplicaDeactivate(const GridMate::ReplicaContext& rc)
    {
        (void)rc;
        if (m_bindingComponent)
        {
            m_bindingComponent->UnbindFromNetwork();
        }
    }

    bool NetBindingComponentChunk::AcceptChangeOwnership(GridMate::PeerId requestor, const GridMate::ReplicaContext& rc)
    {
        bool result = true;

        if (m_bindingComponent)
        {
            EBUS_EVENT_ID_RESULT(result, m_bindingComponent->GetEntityId(), NetBindingEventsBus, OnEntityAcceptChangeOwnership, requestor, rc);
        }

        return result;
    }

    void NetBindingComponentChunk::OnReplicaChangeOwnership(const GridMate::ReplicaContext& rc)
    {
        if (m_bindingComponent)
        {
            EBUS_EVENT_ID(m_bindingComponent->GetEntityId(), NetBindingEventsBus, OnEntityChangeOwnership, rc);
        }
    }
}   // namespace AzFramework
