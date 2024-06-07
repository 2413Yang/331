#ifndef _KANZIUTIL_H_
#define _KANZIUTIL_H_
#include <kanzi/kanzi.hpp>

#define KANZILOOKUPNODE(parent,type,name) parent->lookupNode<type>(name);

#define KANZIGETPREFAB(screen,prefabNode,path) prefabNode = screen->getResourceManager()->tryAcquireResource<PrefabTemplate>(path);

#define KANZIINSTANTIATEPHONELIST(parent,prefabNode,name,propertyName,propertyLocation,propertyTime) Node2DSharedPtr child = prefabNode->instantiate<Node2D>(name);\
												 if(child)\
												 {\
													KANZISETPROPERTY(child,string,"Phone.listName",propertyName)\
													KANZISETPROPERTY(child,string,"Phone.listLocation",propertyLocation)\
													KANZISETPROPERTY(child,string,"Phone.listTime",propertyTime)\
													parent->addItem(child);\
												 }\

#define KANZILOADPREFAB_THEME(screen,parent,name,path,child,prefab) if ((NULL != parent) && (NULL != screen) && (NULL != path) && (NULL != name) )\
										    	 {\
												 	prefab = screen->getResourceManager()->tryAcquireResource<PrefabTemplate>(path);\
													if (prefab != NULL)\
													{\
														child = prefab->instantiate<Node2D>(name);\
														if(child)\
														{\
															parent->addChild(child);\
														}\
													}\
												 }

#define KANZILOADPREFAB(screen,parent,name,path,child) if ((NULL != parent) && (NULL != screen) && (NULL != path) && (NULL != name) )\
												 {\
												 	PrefabTemplateSharedPtr prefab = screen->getResourceManager()->tryAcquireResource<PrefabTemplate>(path);\
													if (prefab != NULL)\
													{\
														child = prefab->instantiate<Node2D>(name);\
														if(child)\
														{\
															parent->addChild(child);\
														}\
													}\
												 }

#define KANZILOADPREFAB_PAGE(screen,parent,name,path,child) if ((NULL != parent) && (NULL != screen) && (NULL != path) && (NULL != name))\
												 {\
												 	PrefabTemplateSharedPtr prefab = screen->getResourceManager()->tryAcquireResource<PrefabTemplate>(path);\
													if (prefab != NULL)\
													{\
														child = prefab->instantiate<Page>(name);\
														if(child)\
														{\
															parent->addChild(child);\
														}\
													}\
												 }

#define KANZISETPROPERTY(node,type,property,value)  node->setProperty(DynamicPropertyType<type>(property), value);

#define KANZIGETPROPERTY(node,type,property) node->getProperty(DynamicPropertyType<type>(property));

#endif // End KANZI_UTIL_H