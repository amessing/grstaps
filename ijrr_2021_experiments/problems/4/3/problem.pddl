(define (problem ijrr_3)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street27 street30 - street
		building13 building17 building18 building19 - building
	)
	(:init
		(atLocation survivor0 street30)
		(atLocation survivor1 building13)
		(onFire building17)
		(onFire building19)
		(underRubble street27)
		(underRubble building18)
		(needsRepair building17)
		(needsRepair building18)
		(needsRepair building19)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street27))
		(not (needsRepair building17))
		(not (needsRepair building18))
		(not (needsRepair building19))
	))
	(:metric minimize total-time)
)
