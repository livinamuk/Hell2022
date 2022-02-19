#include "ContactReportCallback.h"

std::vector<CollisionReport> ContactReportCallback::s_collisionReports;	

void ContactReportCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	CollisionReport report;
	report.rigidA = pairHeader.actors[0];
	report.rigidB = pairHeader.actors[1];
	report.dataA = (EntityData*)pairHeader.actors[0]->userData;
	report.dataB = (EntityData*)pairHeader.actors[1]->userData;

	s_collisionReports.push_back(report);
}