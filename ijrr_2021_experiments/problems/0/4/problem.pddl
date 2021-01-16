(define (problem ijrr_4)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street14 street30 - street
		building3 building13 building17 building32 - building
	)
	(:init
		(atLocation survivor0 street30)
		(atLocation survivor1 building13)
		(onFire building3)
		(onFire building32)
		(underRubble street14)
		(underRubble building17)
		(needsRepair building3)
		(needsRepair building17)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street14))
		(not (needsRepair building3))
		(not (needsRepair building17))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
