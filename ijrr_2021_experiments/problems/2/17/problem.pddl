(define (problem ijrr_17)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street5 street40 - street
		building16 building17 building29 building33 - building
	)
	(:init
		(atLocation survivor0 street40)
		(atLocation survivor1 building33)
		(onFire building16)
		(onFire building29)
		(underRubble street5)
		(underRubble building17)
		(needsRepair building16)
		(needsRepair building17)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street5))
		(not (needsRepair building16))
		(not (needsRepair building17))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
