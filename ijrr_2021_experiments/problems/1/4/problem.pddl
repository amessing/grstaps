(define (problem ijrr_4)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street3 street12 - street
		building8 building10 building17 building25 - building
	)
	(:init
		(atLocation survivor0 street12)
		(atLocation survivor1 building8)
		(onFire building10)
		(onFire building25)
		(underRubble street3)
		(underRubble building17)
		(needsRepair building10)
		(needsRepair building17)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street3))
		(not (needsRepair building10))
		(not (needsRepair building17))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
